#include "Server_Main.h"
#include "Logger.h"
#include "CommandExecuter.h"
#include "Network/AsyncServer.h"
#include "HashHelper.h"
#include "Network/OpCodes.h"
#include "Network/PlayerAuthenticator.h"
#include "MatchManager.h"
#include "Match.h"
#include "Player.h"

#include <boost/json/src.hpp>

using namespace boost;
 
Server_Main* Server_Main::m_instance = nullptr;

void Server_Main::UserConnected(void* socket_usr)
{
	AsyncServer::SocketUser* socket_user = (AsyncServer::SocketUser*)socket_usr;

	Player* player = (Player*)socket_user->GetUser();

	if (socket_user->Has_User() && socket_user->GetUser() != nullptr) {
		Player* player = (Player*)socket_user->GetUser();
		Match* player_match = player->Get_Active_Match();
		if (player_match != nullptr) {
			player_match->RemovePlayer(player);
		}
	}

	Logger::Log("User '" + socket_user->SessionToken + "' has connected.");

	// Request basic user information
	//socket_user->Send(OpCodes::Client::Request_Identity, std::vector<uint8_t>({ 0x01 }));
}

void Server_Main::UserDisconnected(void* socket_usr)
{
	AsyncServer::SocketUser* socket_user = (AsyncServer::SocketUser*)socket_usr;

	Logger::Log("Socket '" + socket_user->SessionToken + "' has disconnected.");

	if (socket_user->GetUser() != nullptr) {
		Player* player = (Player*)socket_user->GetUser();
		std::string username = player->Get_UserName();
		if (Has_Player(player->Get_UserID())) {
			m_players.erase(player->Get_UserID());
		}
		delete player;
		Logger::Log("Player '" + username + "' removed.");
	}
	
}

void Server_Main::PlayerAuthenticated(Player* player, bool authorized)
{
	if (authorized) {
		Logger::Log(player->Get_UserName() + " authenticated successfully!");

		if (Has_Player(player->Get_UserID())) {
			Logger::Log(player->Get_UserName() + " logged on again!");
			player->Socket_User()->Close(true);
			delete player;
		}
		else {
			player->Socket_User()->Set_Authenticated(true);
			m_players[player->Get_UserID()] = player;
		}
	}
	else {
		Logger::Log(player->Get_UserName() + " not authorized.");
		player->Socket_User()->Close(true);
		delete player;
	}
}

Server_Main::Server_Main(char* args)
{
	m_instance = this;

	m_cmdArgs = args;
	frameCounter = 0;
	m_curCommand = "";
	m_executedCommand = "";

	char pBuf[256];
#ifdef WINDOWS_PLATFROM
	size_t len = sizeof(pBuf);
	int bytes = GetModuleFileName(NULL, pBuf, len);
#else // linux
	int bytes = MIN(readlink("/proc/self/exe", pBuf, len), len - 1);
	if (bytes >= 0)
		pBuf[bytes] = '\0';
#endif

	m_app_dir = pBuf;
	m_app_dir = m_app_dir.substr(0, m_app_dir.find_last_of('\\'));

}

void Server_Main::Start()
{
	Init();
	Loop();
}

void Server_Main::LoadSettings(std::string file)
{
}

void Server_Main::Init()
{
	m_running = true;

	m_com_executer = new CommandExecuter();
	m_com_executer->Run(false);

	Logger::Init();
	//Logger::Log("Server Started!");

	m_authenticator = new PlayerAuthenticator(this);
	m_net_server = new AsyncServer(this);

	m_net_server->AddCommand(OpCodes::Server::Submit_Identity, Server_Main::UserIdentify_cb, this);
	m_net_server->AddCommand(OpCodes::Server::Join_Match, Server_Main::JoinMatch_cb, this);

	m_match_manager = new MatchManager();

	Logger::Log("Server Initialized Successfully!");
}

void Server_Main::Loop()
{
	timer.restart();
	lastTime = timer.elapsed();
	while (m_running)
	{
		double curTime = timer.elapsed();
		double dt = curTime - lastTime;
		lastTime = curTime;

		Sleep(1);
		Update(dt);
		frameCounter++;
	}
}

void Server_Main::Update(double dt)
{
	if (!m_running)
		return;

	m_com_executer->Process();
	m_net_server->Update(dt);
	m_match_manager->Update(dt);

	if (m_executedCommand != "")
	{


		// TODO: Add commands.

		m_executedCommand = "";
	}

	Logger::Update();

}

void Server_Main::Stop()
{
	m_running = false;

	Dispose();
}

void Server_Main::Dispose()
{
}

void Server_Main::SetCurrentCommand(std::string command)
{
	m_curCommand = command;
}

void Server_Main::UserIdentify(AsyncServer::SocketUser* user, Data data)
{
	Player* player = new Player();
	user->SetUser((IUser*)player);

	bool is_identified = player->SetIdentity(HashHelper::BytesToString(data.Buffer));

	if (is_identified) {
		m_authenticator->Authenticate(player);
	}
	else {
		user->Close(true);
		delete player;
	}

	uint8_t res = is_identified ? 0x01 : 0x00;

	user->Send(OpCodes::Client::Identify_Result, std::vector<uint8_t>({ res }));
}

void Server_Main::JoinMatch(AsyncServer::SocketUser* user, Data data)
{
	if (!user->Get_Authenticated()) {
		// send fail
		user->Send(OpCodes::Client::Join_Match_Result, std::vector<uint8_t>({ 0x00, 0x00, 0x00 }));
		return;
	}

	Player* player = (Player*)user->GetUser();

	std::string json_string = HashHelper::BytesToString(data.Buffer);

	Logger::Log(json_string);

	json::parse_options opt;
	opt.allow_trailing_commas = true;

	json::error_code ec;
	json::value json_val = json::parse(json_string, ec, json::storage_ptr(), opt);

	if (ec) {
		Logger::Log("Failed to parse match json.");
		// send fail
		user->Send(OpCodes::Client::Join_Match_Result, std::vector<uint8_t>({ 0x00, 0x00, 0x00 }));
		return;
	}

	json::object ident_obj = json_val.as_object();
	std::string match_id = std::string(ident_obj.at("Match_ID").as_string());

	

	bool join_res = m_match_manager->AddMatchPlayer(player, match_id);

	bool res = 0x00;
	uint16_t match_short_id = 0;
	Match* match = m_match_manager->GetMatchFromID(match_id);
	if (match != nullptr && join_res) {
		res = join_res;
		match_short_id = match->ShortID();
		res = 0x01;
	}
	else {
		Logger::Log("Match join failed: " + std::to_string(join_res));
	}



	Logger::Log("Join " + player->Get_UserName() + " To match " + match_id + ", " + std::to_string(match_short_id));

	user->Send(OpCodes::Client::Join_Match_Result, std::vector<uint8_t>({res, ((uint8_t*)&match_short_id)[0], ((uint8_t*)&match_short_id)[1]}));
}
