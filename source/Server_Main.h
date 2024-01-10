#pragma once

#include "stdafx.h"
#include "Network/Data.h"


#include <boost/timer.hpp>

class CommandExecuter;
class Logger;
class AsyncServer;
class SocketUser;
class MatchManager;
class PlayerAuthenticator;
class Player;

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

	MatchManager* m_match_manager;

	PlayerAuthenticator* m_authenticator;

	std::unordered_map<uint32_t, std::shared_ptr<Player>> m_players;

	std::map<std::string, uint64_t> m_memory_usage;
	std::mutex m_memory_lock;
	uint64_t m_last_memory_print_time;

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

	static uint64_t GetMemoryUsage();

	static void SetMemoryUsageForThread(std::string name) {
		SetMemoryUsageForThread(name, GetMemoryUsage());
	}

	static void SetMemoryUsageForThread(std::string name, uint64_t usage) {
		m_instance->m_memory_lock.lock();
		m_instance->m_memory_usage[name] = usage;
		m_instance->m_memory_lock.unlock();
	}

	void SetCurrentCommand(std::string command);

	void SetCommand(std::string command)
	{
		m_executedCommand = command;
	}

	void UserConnected(std::shared_ptr<SocketUser> socket_user);

	void UserDisconnected(SocketUser* socket_user);

	void PlayerAuthenticated(std::shared_ptr<Player>, bool authorized);

	bool Has_Player(uint32_t p_id) {
		return m_players.find(p_id) != m_players.end();
	}

	std::shared_ptr<Player> CreateFakePlayer(uint32_t id);

	Server_Main(char* args);


	void Start();

	void LoadSettings(std::string file);

	void Init();

	void Loop();

	void Update(double dt);

	void Stop();

	void Dispose();

	static void UserIdentify_cb(void* obj, SocketUser& user, Data data) {
		m_instance->UserIdentify(user, data);
	}
	void UserIdentify(SocketUser& user, Data data);

	static void JoinMatch_cb(void* obj, SocketUser& user, Data data) {
		m_instance->JoinMatch(user, data);
	}
	void JoinMatch(SocketUser& user, Data data);
};