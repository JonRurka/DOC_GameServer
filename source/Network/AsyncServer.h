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

#include <boost/asio.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

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
	
	typedef void(*CommandActionPtr)(void*, SocketUser&, Data);

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
	
	void RemovePlayer(std::string user);

	bool HasPlayerSession(std::string session_key);

	void PlayerAuthenticated(std::shared_ptr<SocketUser> user, bool authorized);

	int Get_TCP_Send_Queue_Size_All();

	glm::uvec2 Get_UDP_Sends();

	void Receive_UDP(std::vector<uint8_t> data, boost::asio::ip::address endpoint);

	void Receive_UDP(uint8_t* data, uint16_t size, boost::asio::ip::address endpoint);

	void handle_accept(const boost::system::error_code& error);

	//void Process(SocketUser* socket_user, Data data);
	void Process(SocketUser* socket_user, uint8_t command, uint8_t* data, int size, Protocal type);
	

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
		//Data data;
		uint8_t command;
		std::string user;
		uint8_t* buffer;
		int buffer_size;
		Protocal type;
	};

	static AsyncServer* m_instance;

	bool m_run;
	bool m_run_async_commands;

	PlayerAuthenticator m_authenticator;
	Server_Main* m_server;
	boost::asio::io_service m_io_service_udp;
	boost::asio::io_service m_io_service_tcp;
	tcp_server* m_tcp_server;
	//udp_server* m_udp_server; // TODO: Might be good to make several.
	udp_main_server* m_udp_server;

	std::thread m_thread_1;
	std::thread m_thread_2;

	std::unordered_map<uint8_t, NetCommand> m_commands;
	std::unordered_map<std::string, std::shared_ptr<SocketUser>> m_socket_users;
	std::unordered_map<uint16_t, std::shared_ptr<SocketUser>> m_udp_id_map;
	std::vector<std::shared_ptr<SocketUser>> m_Socket_user_list;
	std::recursive_mutex m_user_mtx;

	std::queue<std::shared_ptr<SocketUser>> m_queue_user_add;
	std::queue<std::string> m_queue_user_remove;

	std::queue<ThreadCommand> m_main_command_queue;
	std::mutex m_main_command_queue_lock;
	int m_main_cmd_q_len = 0;

	std::queue<ThreadCommand> m_async_command_queue;
	std::mutex m_async_command_queue_lock;
	int m_async_cmd_q_len = 0;

	int threadSafeCommandQueueDuplicate(std::mutex& lock, std::queue<ThreadCommand>& from, std::queue<ThreadCommand>& to);

	void PopulateUserList();

	void DoAddPlayer(std::shared_ptr<SocketUser> user);

	void DoRemovePlayer(std::string user);

	void DoProcess(std::string socket_user, Data data);

	static void Process_Async(AsyncServer* svr);

	static void System_Cmd_cb(void* obj_ptr, SocketUser& socket_user, Data data) {
		AsyncServer* svr = (AsyncServer*)obj_ptr;
		svr->System_Cmd(socket_user, data);
	}
	void System_Cmd(SocketUser& socket_user, Data data);

};