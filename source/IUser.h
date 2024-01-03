#pragma once

#include "Network/AsyncServer.h"

class IUser {
	friend class AsyncServer::SocketUser;
private:
	AsyncServer::SocketUser* m_socket_user;
	void Set_Socket_User(AsyncServer::SocketUser* socket_user) {
		m_socket_user = socket_user;
	}

public:
	AsyncServer::SocketUser* Socket_User() {
		return m_socket_user;
	}
};
