#include "Match.h"
#include "Network/AsyncServer.h"
#include "Player.h"
#include "Network/BufferUtils.h"
#include "HashHelper.h"
#include "Server_Main.h"

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

bool Match::JoinPlayer(Player* player)
{
	if (m_match_state != MatchState::Joined_Waiting) {
		Logger::Log("Tried to add player in invalid match state.");
		return false;
	}

	// In a isolated match, join any player,
	// In a network match, only join assigned players.

	player->Set_Active_Match(this);
	m_players[player->Get_UserID()] = player;

	return true;
}

bool Match::RemovePlayer(Player* player)
{
	if (HasPlayer(player)) {
		player->Set_Active_Match(nullptr);
		m_players.erase(player->Get_UserID());
		return true;
	}
	return false;
}

bool Match::HasPlayer(Player* player)
{
	return m_players.find(player->Get_UserID()) != m_players.end();
}

void Match::StartMatch()
{
	if (m_match_state != MatchState::Joined_Waiting) {
		return;
	}

	json::object obj;

	obj["seed"] = 0;
	
	json::array player_arr(m_players.size());

	int i = 0;
	for (auto& pair : m_players) {
		json::object player_obj;

		Player* player = pair.second;

		player_obj["UserName"] = player->Get_UserName();
		player_obj["User_ID"] = player->Get_UserID();
		//player_obj["Instance_ID"] = i;

		player_arr[i] = player_obj;
		i++;
	}

	obj["players"] = player_arr;

	std::string json_str = json::serialize(obj);

	// seed, players, other options
	std::vector<uint8_t> match_start_data = HashHelper::StringToBytes(json_str);

	m_match_state = MatchState::Started;
	BroadcastCommand(OpCodes::Client::Start_Match, match_start_data);

	Logger::Log("Match " + m_ID + " started.");
}

void Match::BroadcastCommand(OpCodes::Client cmd, std::vector<uint8_t> data, Protocal type)
{
	for (auto& pair : m_players) {
		if (pair.second != nullptr) {
			// could crash if player removed at wrong time.
			pair.second->Socket_User()->Send(cmd, data, type);
		}
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
	ProcessNetCommands();
	SendOrientationUpdates();
}

void Match::SendOrientationUpdates()
{
	uint64_t now = Server_Main::GetEpoch();

	if ((now - m_last_orientation_update) > ORIENTATION_SEND_RATE) {




		m_last_orientation_update = Server_Main::GetEpoch();
	}

}

void Match::ProcessNetCommands()
{
	while (!m_command_queue.empty()) {
		NetCommand data = m_command_queue.front();
		m_command_queue.pop();

		ExecuteNetCommand(data.user, data.data);
	}
}

void Match::ExecuteNetCommand(AsyncServer::SocketUser* user, Data data)
{
	OpCodes::Server_Match sub_command = (OpCodes::Server_Match)data.Buffer[0];
	data.Buffer = BufferUtils::RemoveFront(Remove_CMD, data.Buffer);

	switch (sub_command) {
	case OpCodes::Server_Match::Debug_Start:
		StartMatch_NetCmd(user, data);
		break;
	case OpCodes::Server_Match::Update_Orientation:
		UpdateOrientation_NetCmd(user, data);
		break;
	}
}

void Match::StartMatch_NetCmd(AsyncServer::SocketUser* user, Data data)
{
	// check if isolated mode

	StartMatch();
}

void Match::UpdateOrientation_NetCmd(AsyncServer::SocketUser* user, Data data)
{
	float* loc_buff = (float*)data.Buffer.data();
	float* rot_buff = &((float*)data.Buffer.data())[3];

	float loc_x = loc_buff[0];
	float loc_y = loc_buff[1];
	float loc_z = loc_buff[2];

	float rot_x = rot_buff[0];
	float rot_y = rot_buff[1];
	float rot_z = rot_buff[2];
	float rot_w = rot_buff[3];

	glm::vec3 location = glm::vec3(loc_x, loc_y, loc_z);
	glm::quat rotation = glm::quat(rot_w, rot_x, rot_y, rot_z);

	Player* player = (Player*)user->GetUser();

	player->Set_Location(location);
	player->Set_Rotation(rotation);

	//std::string loc_str = "(" + std::to_string(loc_x) + ", " + std::to_string(loc_y) + ", " + std::to_string(loc_z) + ")";
	//std::string rot_str = "(" + std::to_string(rot_x) + ", " + std::to_string(rot_y) + ", " + std::to_string(rot_z) + ", " + std::to_string(rot_w) + ")";
		
	//Logger::Log("Received orientation update: " + loc_str + ", " + rot_str);
}

void Match::SubmitMatchCommand(AsyncServer::SocketUser* user, Data data)
{
	NetCommand command;
	command.user = user;
	command.data = data;
	m_command_queue.push(command);
}
