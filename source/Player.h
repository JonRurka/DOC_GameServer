#pragma once

#include "stdafx.h"
#include "IUser.h"

class Match;

class Player : public IUser {
public:

	Player();
	~Player();

	bool SetIdentity(std::string json_identity);

	std::string Get_UserName() {
		return m_userName;
	}

	std::string Get_Distribution_UserID() {
		return m_distro_userID;
	}

	uint32_t Get_UserID() {
		return m_UserID;
	}

	uint8_t Get_MatchInstanceID() {
		return m_match_instance_id;
	}

	void Set_MatchInstanceID(uint8_t id) {
		m_match_instance_id = id;
	}

	int Get_Distributor() {
		return m_distributor;
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

	std::vector<uint8_t> Serialize_Orientation() {

		float rot_buf[7] = {
			m_location.x,
			m_location.y,
			m_location.z,
			m_rotation.x,
			m_rotation.y,
			m_rotation.z,
			m_rotation.w
		};

		std::vector<uint8_t> or_arr(((uint8_t*)rot_buf), ((uint8_t*)rot_buf) + (7 * sizeof(float)));
		return or_arr;
	}

private:

	std::string m_userName;
	std::string m_distro_userID;
	uint32_t m_UserID;
	int m_distributor;

	uint8_t m_match_instance_id;

	Match* m_active_match;

	glm::vec3 m_location;
	glm::quat m_rotation;

};