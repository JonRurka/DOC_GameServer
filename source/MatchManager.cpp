#include "MatchManager.h"
#include "Server_Main.h"
#include "Match.h"
#include "HashHelper.h"
#include "Network/BufferUtils.h"
#include "Network/AsyncServer.h"
#include "Network/OpCodes.h"

MatchManager* MatchManager::m_instance = nullptr;

MatchManager::MatchManager()
{
	m_instance = this;

	m_last_new_match_request = 0;

	AsyncServer::GetInstance()->AddCommand(OpCodes::Server::Match_Command, MatchManager::RoutMatchNetCommand_cb, this, true);
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

bool MatchManager::AddMatchPlayer(Player* player, std::string match_id)
{
	if (Has_Match_ID(match_id)) {
		return m_matches_IDs[match_id]->JoinPlayer(player);
	}
	else {
		return false;
	}
}

void MatchManager::RoutMatchNetCommand(AsyncServer::SocketUser* user, Data data)
{
	if (!user->Get_Authenticated()) {
		// send fail
		return;
	}

	uint16_t match_short_id = *((uint16_t*)data.Buffer.data());
	data.Buffer = BufferUtils::RemoveFront(2, data.Buffer);

	if (Has_Match_Short_ID(match_short_id)) {
		m_matches[match_short_id]->SubmitMatchCommand(user, data);
	}
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
	if (Has_Match_ID(request.Match_ID)) {
		return;
	}

	uint16_t match_short_id = Get_New_Match_Short_ID();

	Match* match = new Match(request.Match_ID, match_short_id);

	m_matches[match_short_id] = match;
	m_matches_IDs[request.Match_ID] = match;
}

std::vector<MatchManager::MatchCreationRequest> MatchManager::GetNewMatches()
{
	std::vector<MatchManager::MatchCreationRequest> newRequests;

	// This will grab new matches that should be made from the http api.

	MatchCreationRequest match;

	match.Match_ID = "00000000";
	//match.Players.push_back("placeholder_UID");

	newRequests.push_back(match);

	m_last_new_match_request = Server_Main::GetEpoch();

	return newRequests;
}

uint16_t MatchManager::Get_New_Match_Short_ID()
{
	uint16_t newNum = HashHelper::RandomNumber(0, UINT16_MAX);
	if (Has_Match_Short_ID(newNum)) {
		newNum = Get_New_Match_Short_ID();
	}
	return newNum;
}
