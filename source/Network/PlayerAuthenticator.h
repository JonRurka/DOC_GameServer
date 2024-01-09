#pragma once

#include "../stdafx.h"

class Server_Main;
class Player;

class PlayerAuthenticator {
public:
	PlayerAuthenticator() {
		m_initialized = false;
		m_server = nullptr;
	}
	PlayerAuthenticator(Server_Main* server_inst);

	void Authenticate(std::shared_ptr<Player> player);

private:
	bool m_initialized;
	Server_Main* m_server;

	void HasAuthenticatedPlayer(std::shared_ptr<Player> player, bool authorized);
};