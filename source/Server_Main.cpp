#include "Server_Main.h"
#include "Logger.h"
#include "CommandExecuter.h"
#include "Network/AsyncServer.h"
#include "HashHelper.h"
#include "Network/OpCodes.h"

Server_Main* Server_Main::m_instance = nullptr;

void Server_Main::UserConnected(void* socket_usr)
{
	AsyncServer::SocketUser* socket_user = (AsyncServer::SocketUser*)socket_usr;


	Logger::Log("Player '" + socket_user->SessionToken + "' has connected.");

	// Request basic user information
	//socket_user->Send(OpCodes::Client::Request_Identity, std::vector<uint8_t>({ 0x01 }));
}

void Server_Main::UserDisconnected(void* socket_usr)
{
	AsyncServer::SocketUser* socket_user = (AsyncServer::SocketUser*)socket_usr;


	Logger::Log("Player '" + socket_user->SessionToken + "' has disconnected.");
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

	m_net_server = new AsyncServer(this);

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