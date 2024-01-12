#include "Server_Main.h"
#include "Logger.h"
#include "CommandExecuter.h"
#include "Network/AsyncServer.h"
#include "Network/SocketUser.h"
#include "HashHelper.h"
#include "Network/OpCodes.h"
#include "Network/PlayerAuthenticator.h"
#include "MatchManager.h"
#include "Match.h"
#include "IUser.h"
#include "Player.h"

#include <boost/json/src.hpp>

using namespace boost;
 
Server_Main* Server_Main::m_instance = nullptr;
Server_Main::QueueLengths Server_Main::m_queue_lengths{};

void Server_Main::UserConnected(std::shared_ptr<SocketUser> socket_user)
{

	if (socket_user->Has_User() && !socket_user->GetUser().expired()) {
		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(socket_user->GetUser().lock());
		Match* player_match = player->Get_Active_Match();
		if (player_match != nullptr) {
			player_match->RemovePlayer(player);
		}
	}

	Logger::Log("User '" + socket_user->SessionToken + "' has connected.");

	// Request basic user information
	//socket_user->Send(OpCodes::Client::Request_Identity, std::vector<uint8_t>({ 0x01 }));
}

void Server_Main::UserDisconnected(SocketUser* socket_user)
{
	//SocketUser* socket_user = (SocketUser*)socket_usr;

	Logger::Log("Socket '" + socket_user->SessionToken + "' has disconnected.");

	if (!socket_user->GetUser().expired()) {
		Player* player = std::dynamic_pointer_cast<Player>(socket_user->GetUser().lock()).get();
		std::string username = player->Get_UserName();
		if (Has_Player(player->Get_UserID())) {
			m_players.erase(player->Get_UserID());
		}
		Logger::Log("Player '" + username + "' removed.");
	}
	
}

void Server_Main::PlayerAuthenticated(std::shared_ptr<Player> player, bool authorized)
{
	if (authorized) {
		Logger::Log(player->Get_UserName() + " authenticated successfully!");

		if (Has_Player(player->Get_UserID())) {
			Logger::Log(player->Get_UserName() + " logged on again!");
			player->Socket_User()->Close(true);
			//delete player;
		}
		else {
			player->Socket_User()->Set_Authenticated(true);
			m_players[player->Get_UserID()] = player;
		}
	}
	else {
		Logger::Log(player->Get_UserName() + " not authorized.");
		player->Socket_User()->Close(true);
		//delete player;
	}
}

void Server_Main::PrintQueueLengths()
{
	std::string queue_lenths_str = "Network Queue Lengths: \n";

	queue_lenths_str += "\tUDP_SendQeue: " + std::to_string(m_queue_lengths.UDP_SendQeue) + "\n";
	queue_lenths_str += "\tTCP_SendQeue: " + std::to_string(m_queue_lengths.TCP_SendQeue) + "\n";
	queue_lenths_str += "\tMain_ReceiveQueue: " + std::to_string(m_queue_lengths.Main_ReceiveQueue) + "\n";
	queue_lenths_str += "\tAsync_ReceiveQueue: " + std::to_string(m_queue_lengths.Async_ReceiveQueue) + "\n";

	Logger::Log(queue_lenths_str);
}

std::shared_ptr<Player> Server_Main::CreateFakePlayer(uint32_t id)
{
	Player::PlayerIdentity fake_identity;
	fake_identity.UserName = "Fake_User_" + std::to_string(id);
	fake_identity.UserID = id;
	fake_identity.Distributor = 0;
	fake_identity.User_Distro_ID = "fake_distro_id";
	std::shared_ptr<Player> fake_player = std::shared_ptr<Player>(new Player());
	fake_player->SetIdentity(fake_identity);
	fake_player->Set_Location(glm::vec3(10, 5, 1.10));
	fake_player->Set_Rotation(glm::quat());
	return fake_player;
}

Server_Main::Server_Main(char* args)
{
	m_instance = this;

	m_cmdArgs = args;
	frameCounter = 0;
	m_curCommand = "";
	m_executedCommand = "";


	char pBuf[256]{};
	size_t len = sizeof(pBuf);
#ifdef WINDOWS_PLATFROM
	int bytes = GetModuleFileName(NULL, pBuf, len);
#else // linux
	readlink("/proc/self/exe", pBuf, len);

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

	m_last_memory_print_time = GetEpoch();

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

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

	//Logger::Log("Memory Used: " + GetMemoryUsage());

	SetMemoryUsageForThread("main", GetMemoryUsage());

	auto now = GetEpoch();

	if ((now - m_last_memory_print_time) > 1000){
		/*m_memory_lock.lock();
		for (auto& [key, val] : m_memory_usage) {
			//Logger::Log("Thread " + key + ": " + std::to_string((val / 1000.0)) + "KB");
		}
		m_memory_lock.unlock();*/


		PrintQueueLengths();

		glm::uvec2 udp_sends = m_net_server->Get_UDP_Sends();
		Logger::Log("UDP packets sent: " + std::to_string(udp_sends.y) + " out of " + std::to_string(udp_sends.x));

		m_last_memory_print_time = now;
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

uint64_t Server_Main::GetMemoryUsage()
{
	/*MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

	uint64_t physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
	uint64_t physMemUsedByMe = pmc.WorkingSetSize;*/

	return 0;// physMemUsedByMe;
}

void Server_Main::SetCurrentCommand(std::string command)
{
	m_curCommand = command;
}

void Server_Main::UserIdentify(SocketUser& user, Data data)
{
	std::shared_ptr<Player> player = std::make_shared<Player>();
	user.SetUser(std::static_pointer_cast<IUser>(player));

	bool is_identified = player->SetIdentity(HashHelper::BytesToString(data.Buffer));

	if (is_identified) {
		m_authenticator->Authenticate(player);
	}
	else {
		user.Close(true);
		//delete player;
	}

	uint8_t res = is_identified ? 0x01 : 0x00;

	user.Send(OpCodes::Client::Identify_Result, std::vector<uint8_t>({ res }));
}

void Server_Main::JoinMatch(SocketUser& user, Data data)
{
	if (!user.Get_Authenticated()) {
		// send fail
		user.Send(OpCodes::Client::Join_Match_Result, std::vector<uint8_t>({ 0x00, 0x00, 0x00 }));
		return;
	}

	std::shared_ptr<Player> player = Player::Cast_IUser(user.GetUser());

	std::string json_string = HashHelper::BytesToString(data.Buffer);

	Logger::Log(json_string);

	json::parse_options opt;
	opt.allow_trailing_commas = true;

	json::error_code ec;
	json::value json_val = json::parse(json_string, ec, json::storage_ptr(), opt);

	if (ec) {
		Logger::Log("Failed to parse match json.");
		// send fail
		user.Send(OpCodes::Client::Join_Match_Result, std::vector<uint8_t>({ 0x00, 0x00, 0x00 }));
		return;
	}

	json::object ident_obj = json_val.as_object();
	std::string match_id = std::string(ident_obj.at("Match_ID").as_string());

	//m_match_manager->AddMatchPlayer(CreateFakePlayer(0), match_id);

	bool join_res = m_match_manager->AddMatchPlayer(player, match_id);
	

	bool res = 0x00;
	uint16_t match_short_id = 0;
	std::shared_ptr<Match> match = m_match_manager->GetMatchFromID(match_id);
	if (match != nullptr && join_res) {
		res = join_res;
		match_short_id = match->ShortID();
		res = 0x01;
	}
	else {
		Logger::Log("Match join failed: " + std::to_string(join_res));
	}



	Logger::Log("Join " + player->Get_UserName() + " To match " + match_id + ", " + std::to_string(match_short_id));

	user.Send(OpCodes::Client::Join_Match_Result, std::vector<uint8_t>({res, ((uint8_t*)&match_short_id)[0], ((uint8_t*)&match_short_id)[1]}));
}
