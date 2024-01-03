#pragma once

#include "stdafx.h"
#include "Network/Data.h"
#include "Network/AsyncServer.h"

class Match {
public:
	Match(std::string id);
	~Match();

	void Init();

	void Stop();

	void SubmitMatchCommand(AsyncServer::SocketUser* user, Data data);

private:

	struct NetCommand {
	public:
		AsyncServer::SocketUser* user;
		Data data;
	};

	std::string m_ID;

	std::thread m_thread;

	std::queue<NetCommand> m_command_queue;

	bool m_running;

	void GetMatchInfo();

	static void Run(Match* mtch);

	void ThreadInit();

	void GameLoop();

	void AsynUpdate();

	void ProcessNetCommands();

	void ExecuteNetCommand(AsyncServer::SocketUser* user, Data data);
};