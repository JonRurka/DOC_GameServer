#pragma once

#include "stdafx.h"
#include "Network/Data.h"
#include "Network/OpCodes.h"

#define ORIENTATION_SEND_RATE ((1 / 60.0) * 1000) // MS

class SocketUser;
class Player;

class Match {
public:

	enum class MatchState {
		None = 0,
		Joined_Waiting = 1,
		Started = 2,
		Ended = 3
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

	bool JoinPlayer(std::shared_ptr<Player> player);

	bool RemovePlayer(std::shared_ptr<Player> player);

	bool HasPlayer(uint32_t player_id);

	bool HasPlayer(std::shared_ptr<Player> player);

	void StartMatch();

	void EndMatch();

	void BroadcastCommand(OpCodes::Client cmd, std::vector<uint8_t> data, Protocal type = Protocal_Tcp);

	void SubmitMatchCommand(Player* user, Data data);

private:

	struct NetCommand {
	public:
		uint32_t user;
		Data data;
	};

	std::string m_ID;
	uint16_t m_short_ID;

	std::thread m_thread;

	std::map<uint32_t, std::shared_ptr<Player>> m_players;
	std::mutex m_player_mtx;

	std::queue<NetCommand> m_command_queue;

	MatchState m_match_state;

	uint64_t m_last_orientation_update;
	uint64_t m_last_frame;

	uint8_t* m_orientation_send_buffer = nullptr;
	int m_orientation_send_buffer_size = 0;

	bool m_running;

	void SubmitPlayerEvent(Player& user, OpCodes::Player_Events, std::vector<uint8_t> data);

	void GetMatchInfo();

	static void Run(Match* mtch);

	void ThreadInit();

	void GameLoop();

	void AsynUpdate(float dt);

	void UpdatePlayers(float dt);

	void SendOrientationUpdates();

	void SendPlayerEvents();

	void ProcessNetCommands();

	void ExecuteNetCommand(uint32_t user, Data data);

	void StartMatch_NetCmd(Player& user, Data data);

	void UpdateOrientation_NetCmd(Player& user, Data data);
};