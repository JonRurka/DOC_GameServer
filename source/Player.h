#pragma once

#include "stdafx.h"
#include "IUser.h"


#define PLAYER_ORIENTATION_SIZE (7 * sizeof(float))

class Match;

class Player : public IUser {
public:

	struct PlayerIdentity {
	public:
		std::string UserName;
		std::string User_Distro_ID;
		uint32_t UserID;
		uint32_t Distributor;
	};

	struct PlayerEvent {
	public:
		OpCodes::Player_Events Command;
		std::vector<uint8_t> Data;
	};

	Player();
	~Player();

	static std::shared_ptr<Player> Cast_IUser(std::weak_ptr<IUser> user);

	bool SetIdentity(std::string json_identity);

	bool SetIdentity(PlayerIdentity identity);

	void MatchUpdate(float dt);

	std::string Get_UserName() {
		return m_identity.UserName;
	}

	std::string Get_Distribution_UserID() {
		return m_identity.User_Distro_ID;
	}

	uint32_t Get_UserID() {
		return m_identity.UserID;
	}

	uint8_t Get_MatchInstanceID() {
		return m_match_instance_id;
	}

	void Set_MatchInstanceID(uint8_t id) {
		m_match_instance_id = id;
	}

	int Get_Distributor() {
		return m_identity.Distributor;
	}

	void Set_Active_Match(Match* match) {
		m_active_match = match;
	}

	Match* Get_Active_Match() {
		return m_active_match;
	}

	void Set_Location(glm::vec3 location) {
		m_location = location;

	}

	void Set_Rotation(glm::quat rotation) {
		m_rotation = rotation;
	}

	void Add_Player_Event(OpCodes::Player_Events event_cmd) {
		Add_Player_Event(event_cmd, std::vector<uint8_t>());
	}

	void Add_Player_Event(OpCodes::Player_Events event_cmd, std::vector<uint8_t> data) {
		PlayerEvent p_event;
		p_event.Command = event_cmd;
		p_event.Data = data;
		Add_Player_Event(p_event);
	}

	void Add_Player_Event(PlayerEvent p_event) {
		//m_active_event = p_event;

		switch (p_event.Command) {
		case OpCodes::Player_Events::None:
			break;

		case OpCodes::Player_Events::Jump:
			// Just forward command to be re-broadcast by match, if applicable.
			Forward_Player_Event(p_event);
			break;
		}
	}

	void Forward_Player_Event(PlayerEvent p_event) {
		m_active_events.push(p_event);
	}

	std::vector<uint8_t> Serialize_Orientation(uint8_t* out_buff) {

		float rot_buf[7] = {
			m_location.x,
			m_location.y,
			m_location.z,
			m_rotation.x,
			m_rotation.y,
			m_rotation.z,
			m_rotation.w
		};

		memcpy((void*)out_buff, (void*)rot_buf, OrientationSize());

		return std::vector<uint8_t>();

		/*int size = OrientationSize();
		uint8_t* rot_buff_8 = (uint8_t*)rot_buf;

		std::vector<uint8_t> or_arr(rot_buff_8, rot_buff_8 + size);
		return or_arr;*/
	}

	static const int OrientationSize() {
		return PLAYER_ORIENTATION_SIZE;
	}

	int SerializePlayerEvents(std::vector<uint8_t>& events_buff) {

		int num_events = 0;
		while (!m_active_events.empty()) {
			PlayerEvent active_event = m_active_events.front();
			m_active_events.pop();

			if (active_event.Command == OpCodes::Player_Events::None) {
				continue;
			}

			events_buff.push_back(m_match_instance_id);
			events_buff.push_back((uint8_t)active_event.Command);
			events_buff.push_back((uint8_t)active_event.Data.size());

			if (active_event.Data.size() > 0) {
				events_buff = BufferUtils::Add(events_buff, active_event.Data);
			}

			//Logger::Log("Adding player event to send: " + std::to_string((uint8_t)active_event.Command));
			num_events++;
		}
		return num_events;
	}

private:

	//std::string m_userName;
	//std::string m_distro_userID;
	//uint32_t m_UserID;
	//int m_distributor;
	PlayerIdentity m_identity;

	uint8_t m_match_instance_id;

	std::queue<PlayerEvent> m_active_events;

	Match* m_active_match;

	glm::vec3 m_location;
	glm::quat m_rotation;


	uint64_t m_sent_last_jump;
};