#include "PlayerAuthenticator.h"
#include "AsyncServer.h"
#include "../Server_Main.h"
#include "../Player.h"

PlayerAuthenticator::PlayerAuthenticator(Server_Main* server_inst)
{
	m_server = server_inst;
	m_initialized = true;
}

void PlayerAuthenticator::Authenticate(std::shared_ptr<Player> player)
{
	//SocketUser* user = (SocketUser*)socket_user;

	// TODO: Logic to authorize player.

	HasAuthenticatedPlayer(player, true);
}

void PlayerAuthenticator::HasAuthenticatedPlayer(std::shared_ptr<Player> player, bool authorized)
{
	m_server->PlayerAuthenticated(player, authorized);
}
