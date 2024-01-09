#pragma once

#include "stdafx.h"
#include "Network/Data.h"

class SocketUser;
class Match;
class Player;

#define NEW_MATCH_REQUEST_MS 1000

class MatchManager {
	friend class Match;
public:

	struct MatchCreationRequest {
	public:
		std::string Match_ID;
		//std::vector<std::string> Players; // UIDs of players that should be assigned to match.
	};

	MatchManager();
	~MatchManager();

	void Update(float dt);

	bool AddMatchPlayer(std::shared_ptr<Player> player, std::string match_id);

	std::shared_ptr<Match> GetMatchFromID(std::string id) {
		if (Has_Match_ID(id)) {
			return m_matches_IDs[id];
		}
		return nullptr;
	}

	std::shared_ptr<Match> GetMatchFromShortID(uint16_t short_id) {
		if (Has_Match_Short_ID(short_id)) {
			return m_matches[short_id];
		}
		return nullptr;
	}

	static MatchManager* GetInstance() {
		return m_instance;
	}

	static void RoutMatchNetCommand_cb(void* obj, std::shared_ptr<SocketUser> user, Data data) {
		MatchManager* mnger = (MatchManager*)obj;
		mnger->RoutMatchNetCommand(user, data);
	}

	void RoutMatchNetCommand(std::shared_ptr<SocketUser> user, Data data);

private:

	static MatchManager* m_instance;

	uint64_t m_last_new_match_request;

	std::map<uint16_t, std::shared_ptr<Match>> m_matches;
	std::map<std::string, std::shared_ptr<Match>> m_matches_IDs;

	void CreateMatches();

	void CreateMatch(MatchCreationRequest request);

	void RemoveMatch(std::string id);

	uint16_t Get_New_Match_Short_ID();

	bool Has_Match_ID(std::string id) {
		return m_matches_IDs.find(id) != m_matches_IDs.end();
	}

	bool Has_Match_Short_ID(uint16_t id) {
		return m_matches.find(id) != m_matches.end();
	}

	std::vector<MatchCreationRequest> GetNewMatches();
	
	

};