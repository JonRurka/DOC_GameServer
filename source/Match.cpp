#include "Match.h"
#include "Network/AsyncServer.h"
#include "Network/SocketUser.h"
#include "IUser.h"
#include "Player.h"
#include "Network/BufferUtils.h"
#include "HashHelper.h"
#include "Server_Main.h"
#include "MatchManager.h"

#include <boost/json.hpp>

using namespace boost;

Match::Match(std::string id, uint16_t short_id)
{
	m_ID = id;
	m_short_ID = short_id;
	m_match_state = MatchState::None;
}

Match::~Match()
{
}

void Match::Init()
{
	if (m_match_state == MatchState::None) {
		m_thread = std::thread(Run, this);
	}
}

void Match::Stop()
{
	if (m_running) {
		m_running = false;
		m_thread.join();
	}
}

bool Match::JoinPlayer(std::shared_ptr<Player> player)
{
	if (m_match_state != MatchState::Joined_Waiting) {
		Logger::Log("Tried to add player in invalid match state.");
		return false;
	}

	// In a isolated match, join any player,
	// In a network match, only join assigned players.

	m_player_mtx.lock();

	player->Set_Active_Match(this);
	m_players[player->Get_UserID()] = player;
	player->Set_MatchInstanceID(m_players.size());

	m_player_mtx.unlock();

	return true;
}

bool Match::RemovePlayer(std::shared_ptr<Player> player)
{
	m_player_mtx.lock();
	if (HasPlayer(player)) {
		player->Set_Active_Match(nullptr);
		m_players.erase(player->Get_UserID());
		return true;
	}
	m_player_mtx.unlock();
	return false;
}

bool Match::HasPlayer(std::shared_ptr<Player> player)
{
	return m_players.find(player->Get_UserID()) != m_players.end();
}

void Match::StartMatch()
{
	if (m_match_state != MatchState::Joined_Waiting) {
		return;
	}

	uint8_t num_orientations = m_players.size();
	int player_entry_size = (Player::OrientationSize() + 1);
	m_orientation_send_buffer_size = (player_entry_size * num_orientations) + 1;
	m_orientation_send_buffer = new uint8_t[m_orientation_send_buffer_size];

	json::object obj;

	obj["seed"] = 0;
	
	json::array player_arr(m_players.size());

	int i = 0;

	m_player_mtx.lock();
	for (auto& pair : m_players) {
		json::object player_obj;

		std::shared_ptr<Player> player = pair.second;

		player_obj["UserName"] = player->Get_UserName();
		player_obj["User_ID"] = player->Get_UserID();
		player_obj["Instance_ID"] = player->Get_MatchInstanceID();

		player_arr[i] = player_obj;
		i++;
	}
	m_player_mtx.unlock();

	obj["players"] = player_arr;

	std::string json_str = json::serialize(obj);

	// seed, players, other options
	std::vector<uint8_t> match_start_data = HashHelper::StringToBytes(json_str);

	m_match_state = MatchState::Started;
	BroadcastCommand(OpCodes::Client::Start_Match, match_start_data);

	Logger::Log("Match " + m_ID + " started");
}

void Match::EndMatch()
{
	m_match_state = MatchState::Ended;
	Stop();
	MatchManager::GetInstance()->RemoveMatch(m_ID);
}

void Match::BroadcastCommand(OpCodes::Client cmd, std::vector<uint8_t> data, Protocal type)
{
	for (auto& pair : m_players) {
		pair.second->Send(cmd, data, type);
	}
}

void Match::SubmitPlayerEvent(std::shared_ptr<SocketUser> user, OpCodes::Player_Events event_cmd, std::vector<uint8_t> data)
{
	std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(user->GetUser().lock());
	if (player != nullptr && user->Get_Authenticated()) {
		player->Add_Player_Event(event_cmd, data);
	}
}

void Match::GetMatchInfo()
{
}

void Match::Run(Match* mtch)
{
	mtch->ThreadInit();
	mtch->GameLoop();
}

void Match::ThreadInit()
{
	GetMatchInfo();

	m_running = true;
	m_match_state = MatchState::Joined_Waiting;

	m_last_orientation_update = Server_Main::GetEpoch();

	Logger::Log("Match " + m_ID + " Waiting for players...");
}

void Match::GameLoop()
{
	while (m_running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		AsynUpdate();
	}
}

void Match::AsynUpdate()
{
	float dt = 0;

	ProcessNetCommands();
	UpdatePlayers(dt);
	SendOrientationUpdates();
	SendPlayerEvents();

	Server_Main::SetMemoryUsageForThread("match_" + m_ID);
}

void Match::UpdatePlayers(float dt)
{
	m_player_mtx.lock();
	for (auto& pair : m_players) {
		pair.second->MatchUpdate(dt);
	}
	m_player_mtx.unlock();
}

void Match::SendOrientationUpdates()
{
	if (m_match_state != MatchState::Started) {
		return;
	}

	uint64_t now = Server_Main::GetEpoch();

	if ((now - m_last_orientation_update) > ORIENTATION_SEND_RATE) {

		uint8_t num_orientations = m_players.size();

		/*std::vector<uint8_t> send_buff;
		//send_buff.reserve((Player::OrientationSize() + 1) * num_orientations + 1);

		
		m_player_mtx.lock();
		for (auto& pair : m_players) {

			std::shared_ptr<Player> player = pair.second;

			std::vector<uint8_t> player_orient_buff = player->Serialize_Orientation();
			player_orient_buff = BufferUtils::AddFirst(player->Get_MatchInstanceID(), player_orient_buff);

			send_buff = BufferUtils::Add(send_buff, player_orient_buff);
		}
		m_player_mtx.unlock();
		send_buff = BufferUtils::AddFirst(num_orientations, send_buff);*/

		int player_entry_size = (Player::OrientationSize() + 1);

		m_orientation_send_buffer[0] = num_orientations;

		m_player_mtx.lock();
		int p_index = 0;
		for (auto& pair : m_players) {
			Player* player = pair.second.get();

			int buffer_index = (p_index * player_entry_size) + 1;

			m_orientation_send_buffer[buffer_index] = player->Get_MatchInstanceID();
			player->Serialize_Orientation(&m_orientation_send_buffer[buffer_index + 1]);
			
			p_index++;
		}
		m_player_mtx.unlock();

		std::vector<uint8_t> send_buff(m_orientation_send_buffer, m_orientation_send_buffer + m_orientation_send_buffer_size);
		

		BroadcastCommand(OpCodes::Client::Update_Orientations, send_buff); //Protocal_Udp


		m_last_orientation_update = Server_Main::GetEpoch();
	}

}

void Match::SendPlayerEvents()
{
	if (m_match_state != MatchState::Started) {
		return;
	}

	std::vector<uint8_t> send_buff;

	uint8_t num_events = 0;
	m_player_mtx.lock();
	for (auto& pair : m_players) {
		std::shared_ptr<Player> player = pair.second;

		num_events += player->SerializePlayerEvents(send_buff);
	}
	m_player_mtx.unlock();

	if (num_events == 0) {
		return;
	}


	//Logger::Log("Broadcast events: " + std::to_string(num_events));
	send_buff = BufferUtils::AddFirst(num_events, send_buff);
	BroadcastCommand(OpCodes::Client::Player_Events, send_buff);

}

void Match::ProcessNetCommands()
{
	while (!m_command_queue.empty()) {
		NetCommand data = m_command_queue.front();
		m_command_queue.pop();

		ExecuteNetCommand(data.user, data.data);
		
	}
}

void Match::ExecuteNetCommand(std::shared_ptr<SocketUser> user, Data data)
{
	
	if (data.Buffer.size() > 0) {
		OpCodes::Server_Match sub_command = (OpCodes::Server_Match)data.Buffer[0];
		data.Buffer = BufferUtils::RemoveFront(Remove_CMD, data.Buffer);

		//Logger::Log("Received match command: " + std::to_string((uint8_t)sub_command));

		switch (sub_command) {
		case OpCodes::Server_Match::Debug_Start:
			StartMatch_NetCmd(user, data);
			break;
		case OpCodes::Server_Match::Update_Orientation:
			UpdateOrientation_NetCmd(user, data);
			break;
		case OpCodes::Server_Match::Player_Event:
			if (data.Buffer.size() > 0)
			{
				OpCodes::Player_Events event_cmd = (OpCodes::Player_Events)data.Buffer[0];
				data.Buffer = BufferUtils::RemoveFront(Remove_CMD, data.Buffer);
				SubmitPlayerEvent(user, event_cmd, data.Buffer);
			}
			else {

				Logger::LogWarning("Received malformed Match Player Event from '" + Player::Cast_IUser(user->GetUser())->Get_UserName() + "'!");
			}
			break;
		}
	}
	else {
		Logger::LogWarning("Received malformed Match Net Command from '" + Player::Cast_IUser(user->GetUser())->Get_UserName() + "'!");
	}
}

void Match::StartMatch_NetCmd(std::shared_ptr<SocketUser> user, Data data)
{
	// check if isolated mode

	StartMatch();
}

void Match::UpdateOrientation_NetCmd(std::shared_ptr<SocketUser> user, Data data)
{
	//return;

	float* loc_buff = (float*)data.Buffer.data();
	//float* rot_buff = &((float*)data.Buffer.data())[3];

	float loc_x = loc_buff[0];
	float loc_y = loc_buff[1];
	float loc_z = loc_buff[2];

	float rot_x = loc_buff[3];
	float rot_y = loc_buff[4];
	float rot_z = loc_buff[5];
	float rot_w = loc_buff[6];

	glm::vec3 location = glm::vec3(loc_x, loc_y, loc_z);
	glm::quat rotation = glm::quat(rot_w, rot_x, rot_y, rot_z);

	std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(user->GetUser().lock());

	player->Set_Location(location);
	player->Set_Rotation(rotation);

	std::string loc_str = "(" + std::to_string(loc_x) + ", " + std::to_string(loc_y) + ", " + std::to_string(loc_z) + ")";
	std::string rot_str = "(" + std::to_string(rot_x) + ", " + std::to_string(rot_y) + ", " + std::to_string(rot_z) + ", " + std::to_string(rot_w) + ")";
		
	//Logger::Log("Received orientation update ("+std::to_string(data.Buffer.size()) + "): " + loc_str + ", " + rot_str + ", " + std::to_string(data.Type));
}

void Match::SubmitMatchCommand(std::shared_ptr<SocketUser> user, Data data)
{
	NetCommand command;
	command.user = user;
	command.data = data;
	m_command_queue.push(command);
}
