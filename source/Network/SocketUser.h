#pragma once

#include "../stdafx.h"
#include "TCP_Connection.h"

class AsyncServer;

class SocketUser : public std::enable_shared_from_this<SocketUser> {
	friend class tcp_connection;
	friend class AsyncServer;

public:
	std::weak_ptr<IUser> User;
	bool has_user;
	std::string SessionToken;
	int Permission;
	bool Connected;
	bool Receiving;
	bool UdpEnabled;
	uint16_t UdpID;
	bool CloseMessage;
	tcp_connection::pointer tcp_connection_client;

	tcp::endpoint TcpEndPoint;
	udp::endpoint UdpEndPoint;

	uint64_t m_last_ping;

private:
	AsyncServer* _server;
	boost::timer timeOutWatch;
	bool IsAuthenticated;

	//udp::socket send_socket_;

public:

	SocketUser() {
		Connected = false;
		has_user = false;
		UdpID = 0;
	}

	SocketUser(AsyncServer* server, tcp_connection::pointer client);

	void Update(float dt);

	void HandleStartConnect();

	void HandleStartConnect_Finished(bool successfull);

	void EnableUdp(int port);

	void SetUser(std::shared_ptr<IUser> user);

	bool Get_Authenticated() {
		return IsAuthenticated;
	}

	void Set_Authenticated(bool val) {
		IsAuthenticated = val;
	}

	std::weak_ptr<IUser> GetUser() {
		return User;
	}

	bool Has_User() {
		return has_user;
	}

	void Set_UDP_ID(uint16_t id) {
		UdpID = id;
	}

	uint16_t Get_UDP_ID() {
		return UdpID;
	}

	void Set_Client_UDP_Port(uint16_t port);

	void Send(OpCodes::Client command, std::string message, Protocal type = Protocal_Tcp);

	void Send(OpCodes::Client cmd, std::vector<uint8_t> data, Protocal type = Protocal_Tcp);

	std::string GetIP();

	void Close(bool sendClose, std::string reason = "");

	void ProcessReceiveBuffer(std::vector<uint8_t> buffer, Protocal type);

	void ResetPingCounter();

private:

	void Send(std::vector<uint8_t> data, Protocal type = Protocal_Tcp);

	void DoSendTcp(std::vector<uint8_t> data);

	void DoSendUdp(std::vector<uint8_t> data);

}; // End SocketUser