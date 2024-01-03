#include "PlayerAuthenticator.h"
#include "AsyncServer.h"
#include "../Server_Main.h"
#include "../Player.h"

PlayerAuthenticator::PlayerAuthenticator(Server_Main* server_inst)
{
	m_server = server_inst;
	m_initialized = true;
}

void PlayerAuthenticator::Authenticate(Player* player)
{
	//AsyncServer::SocketUser* user = (AsyncServer::SocketUser*)socket_user;

	// TODO: Logic to authorize player.

	HasAuthenticatedPlayer(player, true);
}

void PlayerAuthenticator::HasAuthenticatedPlayer(Player* player, bool authorized)
{
	m_server->PlayerAuthenticated(player, authorized);
}
