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
class SocketUser;

#define TIMEOUT_MS 10000
#define RUN_ASYNC_COMMANDS true

#define TCP_PORT 11010
#define UDP_PORT 11011

#define SERVER_LOCAL true

class AsyncServer {
	friend class SocketUser;
public:
	
	typedef void(*CommandActionPtr)(void*, std::shared_ptr<SocketUser>, Data);

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

	void Add_UDP_ID(uint16_t id, std::shared_ptr<SocketUser> user) {
		m_udp_id_map[id] = user;
	}

	void AddPlayer(std::shared_ptr<SocketUser> user);
	
	void RemovePlayer(std::shared_ptr<SocketUser> user);

	bool HasPlayerSession(std::string session_key);

	void PlayerAuthenticated(std::shared_ptr<SocketUser> user, bool authorized);

	static void Test_Client();
	static void Test_Server(void* obj);

	void Receive_UDP(std::vector<uint8_t> data, boost::asio::ip::address endpoint);

	void handle_accept(const boost::system::error_code& error);

	void Process(std::shared_ptr<SocketUser> socket_user, Data data);
	
	

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
		std::weak_ptr<SocketUser> user;
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

	std::unordered_map<uint8_t, NetCommand> m_commands;
	std::unordered_map<std::string, std::shared_ptr<SocketUser>> m_socket_users;
	std::unordered_map<uint16_t, std::shared_ptr<SocketUser>> m_udp_id_map;
	std::recursive_mutex m_user_mtx;

	std::queue<ThreadCommand> m_main_command_queue;
	std::queue<ThreadCommand> m_async_command_queue;

	void DoProcess(std::shared_ptr<SocketUser> socket_user, Data data);

	static void Process_Async(AsyncServer* svr);

	static void System_Cmd_cb(void* obj_ptr, std::shared_ptr<SocketUser> socket_user, Data data) {
		AsyncServer* svr = (AsyncServer*)obj_ptr;
		svr->System_Cmd(socket_user, data);
	}
	void System_Cmd(std::shared_ptr<SocketUser> socket_user, Data data);

};