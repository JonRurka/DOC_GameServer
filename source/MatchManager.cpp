#include "MatchManager.h"
#include "Server_Main.h"

MatchManager::MatchManager()
{
	m_last_new_match_request = 0;
}

MatchManager::~MatchManager()
{
}

void MatchManager::Update(float dt)
{
	auto now = Server_Main::GetEpoch();

	if ((now - m_last_new_match_request) > NEW_MATCH_REQUEST_MS) {
		CreateMatches();
	}

}

void MatchManager::RoutMatchNetCommand(AsyncServer::SocketUser user, Data data)
{
	// Get match ID.
	// Route command to match.
}

void MatchManager::CreateMatches()
{
	std::vector<MatchManager::MatchCreationRequest> match_requests = GetNewMatches();

	for (int i = 0; i < match_requests.size(); i++) {
		CreateMatch(match_requests[i]);
	}
}

void MatchManager::CreateMatch(MatchCreationRequest request)
{

}

std::vector<MatchManager::MatchCreationRequest> MatchManager::GetNewMatches()
{
	std::vector<MatchManager::MatchCreationRequest> newRequests;

	// This will grab new matches that should be made from the http api.

	MatchCreationRequest match;

	match.Match_ID = "placeholder";
	//match.Players.push_back("placeholder_UID");

	newRequests.push_back(match);

	m_last_new_match_request = Server_Main::GetEpoch();

	return newRequests;
}
