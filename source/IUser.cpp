#include "IUser.h"
#include "Network/SocketUser.h"

void IUser::Send(OpCodes::Client cmd, std::vector<uint8_t> message, Protocal type)
{
	if (Connected()) {
		m_socket_user->Send(cmd, message, type);
	}
}
