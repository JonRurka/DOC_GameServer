#pragma once

#include "stdafx.h"
#include "Network/Data.h"
#include "Network/AsyncServer.h"

class Match;

#define NEW_MATCH_REQUEST_MS 1000

class MatchManager {
public:

	struct MatchCreationRequest {
	public:
		std::string Match_ID;
		//std::vector<std::string> Players; // UIDs of players that should be assigned to match.
	};

	MatchManager();
	~MatchManager();

	void Update(float dt);

	bool AddMatchPlayer(Player* player, std::string match_id);

	Match* GetMatchFromID(std::string id) {
		if (Has_Match_ID(id)) {
			return m_matches_IDs[id];
		}
		return nullptr;
	}

	Match* GetMatchFromShortID(uint16_t short_id) {
		if (Has_Match_Short_ID(short_id)) {
			return m_matches[short_id];
		}
		return nullptr;
	}

	static void RoutMatchNetCommand_cb(void* obj, AsyncServer::SocketUser* user, Data data) {
		MatchManager* mnger = (MatchManager*)obj;
		mnger->RoutMatchNetCommand(user, data);
	}

	void RoutMatchNetCommand(AsyncServer::SocketUser* user, Data data);

private:

	static MatchManager* m_instance;

	uint64_t m_last_new_match_request;

	std::map<uint16_t, Match*> m_matches;
	std::map<std::string, Match*> m_matches_IDs;

	void CreateMatches();

	void CreateMatch(MatchCreationRequest request);

	uint16_t Get_New_Match_Short_ID();

	bool Has_Match_ID(std::string id) {
		return m_matches_IDs.find(id) != m_matches_IDs.end();
	}

	bool Has_Match_Short_ID(uint16_t id) {
		return m_matches.find(id) != m_matches.end();
	}

	std::vector<MatchCreationRequest> GetNewMatches();
	
	static MatchManager* GetInstance() {
		return m_instance;
	}

};