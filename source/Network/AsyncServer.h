#pragma once

#include "../stdafx.h"
#include "TCP_Connection.h"
#include "TCP_Server.h"
#include "UDP_Server.h"
#include "Data.h"
#include "BufferUtils.h"
#include "PlayerAuthenticator.h"
#include "../Logger.h"
#include "OpCodes.h"

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::ip::address;

class IUser;
class Server_Main;

#define TIMEOUT_MS 10000
#define RUN_ASYNC_COMMANDS true

#define TCP_PORT 11010
#define UDP_PORT 11011

#define SERVER_LOCAL true

class AsyncServer {
public:
	class SocketUser {
		friend class tcp_connection;

	public:
		IUser* User;
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

		SocketUser(AsyncServer* server, tcp_connection::pointer client);

		void Update(float dt);

		void HandleStartConnect();

		void HandleStartConnect_Finished(bool successfull);

		void EnableUdp(int port);

		void SetUser(IUser* user);

		bool Get_Authenticated() {
			return IsAuthenticated;
		}

		void Set_Authenticated(bool val) {
			IsAuthenticated = val;
		}

		IUser* GetUser() {
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
	
	typedef void(*CommandActionPtr)(void*, AsyncServer::SocketUser*, Data);

	AsyncServer(Server_Main* server);

	void Update(float dt);

	void AddCommand(OpCodes::Server cmd, CommandActionPtr callback, void* obj, bool async = false);

	bool HasCommand(uint8_t cmd) {
		return m_commands.find(cmd) != m_commands.end();
	}

	bool Has_UDP_ID(uint16_t udp_id) {
		return m_udp_id_map.find(udp_id) != m_udp_id_map.end();
	}
	
	uint16_t Get_New_UDP_ID();

	void Add_UDP_ID(uint16_t id, SocketUser* user) {
		m_udp_id_map[id] = user;
	}

	void AddPlayer(SocketUser* user);
	
	void RemovePlayer(SocketUser* user);

	bool HasPlayerSession(std::string session_key);

	void PlayerAuthenticated(SocketUser* user, bool authorized);

	static void Test_Client();
	static void Test_Server(void* obj);

	void Receive_UDP(std::vector<uint8_t> data, boost::asio::ip::address endpoint);

	void handle_accept(const boost::system::error_code& error);

	void Process(SocketUser* socket_user, Data data);
	
	

	static AsyncServer* GetInstance() {
		return m_instance;
	}

private:

	struct NetCommand {
		void* Obj_Ptr;
		CommandActionPtr Callback;
		bool Is_Async;
	};

	struct ThreadCommand {
		Data data;
		SocketUser* user = nullptr;
	};

	static AsyncServer* m_instance;

	bool m_run;
	bool m_run_async_commands;

	PlayerAuthenticator m_authenticator;
	Server_Main* m_server;
	boost::asio::io_service m_io_service_udp;
	boost::asio::io_service m_io_service_tcp;
	tcp_server* m_tcp_server;
	udp_server* m_udp_server; // TODO: Might be good to make several.
	

	std::thread m_thread_1;
	std::thread m_thread_2;

	std::map<uint8_t, NetCommand> m_commands;
	std::map<std::string, SocketUser*> m_socket_users;
	std::map<uint16_t, SocketUser*> m_udp_id_map;

	std::queue<ThreadCommand> m_main_command_queue;
	std::queue<ThreadCommand> m_async_command_queue;

	void DoProcess(SocketUser* socket_user, Data data);

	static void Process_Async(AsyncServer* svr);

	static void System_Cmd_cb(void* obj_ptr, AsyncServer::SocketUser* socket_user, Data data) {
		AsyncServer* svr = (AsyncServer*)obj_ptr;
		svr->System_Cmd(socket_user, data);
	}
	void System_Cmd(AsyncServer::SocketUser* socket_user, Data data);

};