#pragma once

#include "stdafx.h"
#include "Network/Data.h"
#include "Network/AsyncServer.h"

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

	static void RoutMatchNetCommand_cb(AsyncServer::SocketUser user, Data data, void* obj) {

	}

	void RoutMatchNetCommand(AsyncServer::SocketUser user, Data data);

private:

	uint64_t m_last_new_match_request;

	void CreateMatches();

	void CreateMatch(MatchCreationRequest request);

	std::vector<MatchCreationRequest> GetNewMatches();
	
};