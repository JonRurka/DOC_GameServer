#include "PlayerAuthenticator.h"
#include "AsyncServer.h"

PlayerAuthenticator::PlayerAuthenticator(AsyncServer* server_inst)
{
	m_server = server_inst;
	m_initialized = true;
}

void PlayerAuthenticator::Authenticate(void* socket_user)
{
	AsyncServer::SocketUser* user = (AsyncServer::SocketUser*)socket_user;

	// TODO: Logic to authorize player.

	HasAuthenticatedPlayer(user, true);
}

void PlayerAuthenticator::HasAuthenticatedPlayer(void* socket_user, bool authorized)
{
	m_server->PlayerAuthenticated((AsyncServer::SocketUser*)socket_user, authorized);
}
