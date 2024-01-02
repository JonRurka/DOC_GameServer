#pragma once

#include "stdafx.h"

class CommandExecuter;
class Logger;
class AsyncServer;

class Server_Main {
private:
	static Server_Main* m_instance;

	std::string m_cmdArgs;
	std::string m_app_dir;
	bool m_running;

	CommandExecuter* m_com_executer;
	std::string m_curCommand;
	std::string m_executedCommand;

	int frameCounter;
	boost::timer timer;
	double lastTime;

	AsyncServer* m_net_server;

public:

	static Server_Main* Instance() {
		return m_instance;
	}

	static CommandExecuter* GetInputProcessor()
	{
		return m_instance->m_com_executer;
	}

	static char sepChar() {
#ifdef WINDOWS_PLATFROM
		return '\\';
#else // linux
		return '/';
#endif
	}

	static std::string AppDirectory() {
		return m_instance->m_app_dir;
	}

	static std::string CmdArgs()
	{
		return m_instance->m_cmdArgs;
	}

	static bool Running()
	{
		return m_instance->m_running;
	}

	static double Elapsed() {
		return m_instance->timer.elapsed();
	}

	static uint64_t GetEpoch() {
		auto now = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
		return duration.count();
	}

	void SetCurrentCommand(std::string command);

	void SetCommand(std::string command)
	{
		m_executedCommand = command;
	}

	void UserConnected(void* socket_user);

	void UserDisconnected(void* socket_user);

	Server_Main(char* args);


	void Start();

	void LoadSettings(std::string file);

	void Init();

	void Loop();

	void Update(double dt);

	void Stop();

	void Dispose();

};