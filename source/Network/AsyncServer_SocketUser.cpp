#include "AsyncServer.h"
#include "TCP_Connection.h"
#include "TCP_Server.h"
#include "../Logger.h"
#include "../HashHelper.h"
#include "../Server_Main.h"
#include "../IUser.h"

AsyncServer::SocketUser::SocketUser(AsyncServer* server, tcp_connection::pointer client) 
{
	int udp_client_port = UDP_PORT;
	if (SERVER_LOCAL) {
		udp_client_port += 1;
	}

	_server = server;
	SessionToken = HashHelper::RandomKey(8);
	tcp_connection_client = client;
	TcpEndPoint = client->socket().remote_endpoint();
	UdpEndPoint = udp::endpoint(TcpEndPoint.address(), udp_client_port);
	timeOutWatch.restart();
	UdpID = 0;
	Permission = 0;
	Connected = true;
	IsAuthenticated = false;
	UdpEnabled = false;
	User = nullptr;
	has_user = false;

	ResetPingCounter();

	tcp_connection_client->Set_Socket_User(this);
}

void AsyncServer::SocketUser::Update(float dt)
{
	auto now = Server_Main::GetEpoch();

	if ((now - m_last_ping) > TIMEOUT_MS) {
		_server->RemovePlayer(this);
	}
}

void AsyncServer::SocketUser::HandleStartConnect()
{
	tcp_connection_client->Start_Initial_Connect();
}

void AsyncServer::SocketUser::HandleStartConnect_Finished(bool successfull)
{
	uint8_t result = 0x00;

	if (successfull) {
		result = 0x01;

		uint16_t udp_id = _server->Get_New_UDP_ID();
		Set_UDP_ID(udp_id);
		_server->Add_UDP_ID(udp_id, this);
		uint8_t* udp_buf = (uint8_t*)&udp_id;
		EnableUdp(UDP_PORT);

		//user.UdpID = udpid;
		//AddUdpID(udpid, user.SessionToken);
		//udpidBuff = BitConverter.GetBytes(udpid);
		//ServerBase.BaseInstance.UserConnected(user);

		//user.Send(0xff, BufferUtils.Add(new byte[]{ 0x01, result }, udpidBuff));

		Logger::Log("UDP ID: " + std::to_string(udp_id));

		Send(OpCodes::Client::System_Reserved, BufferUtils::Add({ {0x01, result}, {udp_buf[0], udp_buf[1]}}), Protocal_Tcp);

		_server->AddPlayer(this);

		// handle messages
		tcp_connection_client->Start_Read();
	}
	else {
		Send(OpCodes::Client::System_Reserved, BufferUtils::Add({ {0x01, result}, {0x00, 0x00} }), Protocal_Tcp);

		//user.Send(0xff, BufferUtils.Add(new byte[]{ 0x01, result }, udpidBuff));
		// dipose socket user
	}
}

void AsyncServer::SocketUser::EnableUdp(int port)
{
	//UdpEndPoint.port() = port;
	//Logger.Log("UDP end point: {0}:{1}", udpEndPoint.Address.ToString(), udpEndPoint.Port);
	UdpEnabled = true;
}

void AsyncServer::SocketUser::SetUser(IUser* user)
{
	if (user != nullptr)
	{
		User = user;
		User->Set_Socket_User(this);
		has_user = true;
		Logger::Log("Set user");
		//User.SetSocket(this);
	}
	else
	{
		Logger::Log("IUser null");
	}
}

void AsyncServer::SocketUser::Set_Client_UDP_Port(uint16_t port)
{
	Logger::Log("Set client UDP port to " + std::to_string((int)port));
	UdpEndPoint.port(port);
	//UdpEndPoint = udp::endpoint(TcpEndPoint.address(), port);
}

void AsyncServer::SocketUser::Send(OpCodes::Client command, std::string message, Protocal type)
{
	std::vector<uint8_t> msg(message.begin(), message.end());
	Send(command, msg, type);
}

void AsyncServer::SocketUser::Send(OpCodes::Client cmd, std::vector<uint8_t> data, Protocal type)
{
	Send(BufferUtils::AddFirst((uint8_t)cmd, data), type);
}

void AsyncServer::SocketUser::Send(std::vector<uint8_t> data, Protocal type)
{
	if (!Connected)
		return;

	//try
	//{
	if (type == Protocal_Tcp || !UdpEnabled)
	{
		DoSendTcp(data);
		//if (_stream != null && data != null)
		//	_stream.SendMessasge(data);
	}
	else
	{
		DoSendUdp(data);
		//_server.SendUdp(data, UdpEndPoint);
	}
	/*}
	catch (IOException)
	{
		Close(false, "Send IOException");
	}
	catch (SocketException)
	{
		Close(false, "Send SocketException");
	}
	catch (Exception ex)
	{
		Logger.LogError("{0}: {1}\n{2}", ex.GetType(), ex.Message, ex.StackTrace);
	}*/
}

void AsyncServer::SocketUser::DoSendTcp(std::vector<uint8_t> data)
{
	if (!Connected)
		return;

	tcp_connection_client->Send(data);
}

void AsyncServer::SocketUser::DoSendUdp(std::vector<uint8_t> data)
{
	if (!Connected)
		return;

	data = BufferUtils::Add_UDP_ID(UdpID, data);

	_server->m_udp_server->Send(UdpEndPoint, data);
}

std::string AsyncServer::SocketUser::GetIP()
{
	address addr = TcpEndPoint.address();
	return addr.to_string();
}

void AsyncServer::SocketUser::Close(bool sendClose, std::string reason)
{
	if (Connected)
	{
		Connected = false;

		if (User != NULL)
		{
			//User.Disconnected();
		}

		tcp_connection_client->close();

		_server->RemovePlayer(this);

		//if (CloseMessage)
		//	Logger::Log("{0}: closed {1}", SessionToken, reason != "" ? "- " + reason : "");
		//_server.RemoveUdpID(UdpID);
	}
}

void AsyncServer::SocketUser::ProcessReceiveBuffer(std::vector<uint8_t> buffer, Protocal type)
{
	timeOutWatch.restart();

	if (buffer.size() > 0)
	{
		uint8_t command = buffer[0];
		buffer = BufferUtils::RemoveFront(Remove_CMD, buffer);
		Data data(type, command, buffer);
		_server->Process(this, data);
	}
	else {
		Logger::Log(std::to_string(type) + ": Received empty buffer!");
	}
}

void AsyncServer::SocketUser::ResetPingCounter()
{
	m_last_ping = Server_Main::GetEpoch();
}
