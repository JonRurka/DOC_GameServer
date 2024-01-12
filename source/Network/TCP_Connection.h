#pragma once
#include "../stdafx.h"
#include "../Logger.h"

class AsyncServer;
class SocketUser;

#include <boost/asio.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <semaphore>

using boost::asio::ip::tcp;
using boost::asio::ip::address;

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
	
private:
	tcp::socket socket_;
	//std::map<int, std::vector<char>> send_buff;
	int sent;
	uint8_t length_buff[2]{};
	std::map<uint64_t, uint8_t*> send_buffers;
	int numSends = 0;
	std::weak_ptr<SocketUser> socket_user;
	std::vector<std::shared_ptr<SocketUser>> tmp_socket_ref;
	std::mutex m_lock;

	uint8_t m_send_buffer[UINT16_MAX];
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_service& io_service)
	{
		return pointer(new tcp_connection(io_service));
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void Set_Socket_User(std::weak_ptr<SocketUser> p_socket_user) {
		socket_user = p_socket_user;
	}

	void Send(uint8_t* sending, size_t len);

	void Send(std::vector<uint8_t> sending);

	void Start_Read();

	void Start_Initial_Connect(std::shared_ptr<SocketUser> p_socket_user);

	void close();

	int GetSendQeueLength() {
		return m_send_queue_len;
	}

private:
	tcp_connection(boost::asio::io_service& io_service)
		: socket_(io_service)
	{
		sent = 0;
		m_send_queue_len = 0;
		m_running = true;
		m_thread_sends = std::thread(RunSend, this);
		//start_send();
	}

	void handle_write(const boost::system::error_code&, size_t transfered);

	void Handle_Initial_Connect(
		const boost::system::error_code&, 
		size_t transfered,
		SocketUser* p_socket_user);

	static void RunSend(tcp_connection* srv) {
		while (srv->m_running) {
			srv->start_send();
			srv->m_sends_semaphore_1.acquire();
		}
	}

	void start_send();

	void handle_read(const boost::system::error_code&, size_t transfered);

	struct Send_Message {
		std::vector<uint8_t> sending;
	};

	std::queue<Send_Message> m_send_messages;
	bool m_running;
	std::thread m_thread_sends;

	int m_send_queue_len;

	std::binary_semaphore m_sends_semaphore_1{ 0 };
	std::binary_semaphore m_sends_semaphore_2{ 0 };
	std::mutex m_send_lock;
};