#pragma once

#include "stdafx.h"
#include "Network/Data.h"
#include "Network/AsyncServer.h"

#define ORIENTATION_SEND_RATE 100 // MS

class Match {
public:

	enum class MatchState {
		None = 0,
		Joined_Waiting = 1,
		Started = 2
	};

	Match(std::string id, uint16_t short_id);
	~Match();

	std::string ID() {
		return m_ID;
	}

	uint16_t ShortID() {
		return m_short_ID;
	}

	void Init();

	void Stop();

	bool JoinPlayer(Player* player);

	bool RemovePlayer(Player* player);

	bool HasPlayer(Player* player);

	void StartMatch();

	void BroadcastCommand(OpCodes::Client cmd, std::vector<uint8_t> data, Protocal type = Protocal_Tcp);

	void SubmitMatchCommand(AsyncServer::SocketUser* user, Data data);

private:

	struct NetCommand {
	public:
		AsyncServer::SocketUser* user;
		Data data;
	};

	std::string m_ID;
	uint16_t m_short_ID;

	std::thread m_thread;

	std::map<uint32_t, Player*> m_players;

	std::queue<NetCommand> m_command_queue;

	MatchState m_match_state;

	uint64_t m_last_orientation_update;

	bool m_running;

	void GetMatchInfo();

	static void Run(Match* mtch);

	void ThreadInit();

	void GameLoop();

	void AsynUpdate();

	void SendOrientationUpdates();

	void ProcessNetCommands();

	void ExecuteNetCommand(AsyncServer::SocketUser* user, Data data);

	void StartMatch_NetCmd(AsyncServer::SocketUser* user, Data data);

	void UpdateOrientation_NetCmd(AsyncServer::SocketUser* user, Data data);
};