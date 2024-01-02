#pragma once

#include "../stdafx.h"

class AsyncServer;

class PlayerAuthenticator {
public:
	PlayerAuthenticator() {
		m_initialized = false;
		m_server = nullptr;
	}
	PlayerAuthenticator(AsyncServer* server_inst);

	void Authenticate(void* socket_user);

private:
	bool m_initialized;
	AsyncServer* m_server;

	void HasAuthenticatedPlayer(void* socket_user, bool authorized);
};