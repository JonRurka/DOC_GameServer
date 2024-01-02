#pragma once

#include "stdafx.h"
#include "Network/Data.h"


class Match {
public:
	Match(std::string id);
	~Match();

	void Init();

	void Stop();

	void SubmitMatchCommand(Data data);

private:

	std::string m_ID;

	std::thread m_thread;

	std::queue<Data> m_command_queue;

	bool m_running;

	void GetMatchInfo();

	static void Run(Match* mtch);

	void ThreadInit();

	void GameLoop();

	void AsynUpdate();

	void ProcessNetCommands();

	void ExecuteNetCommand(Data data);
};