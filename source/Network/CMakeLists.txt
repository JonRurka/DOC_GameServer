set(module "Network")

set(${module}_headers
	AsyncServer.h
	SocketUser.h
	TCP_Connection.h
	udp_connection.h
	TCP_Server.h
	UDP_Server.h
	BufferUtils.h
	Data.h
	PlayerAuthenticator.h
	OpCodes.h
)

set(${module}_sources
	AsyncServer.cpp
	AsyncServer_SocketUser.cpp
	TCP_Connection.cpp
	udp_connection.cpp
	TCP_Server.cpp
	UDP_Server.cpp
	PlayerAuthenticator.cpp
)

add_library(${module} STATIC ${${module}_headers} ${${module}_sources})

