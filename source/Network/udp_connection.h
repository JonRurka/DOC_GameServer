#pragma once

#include "../stdafx.h"
#include "../Logger.h"

#include <boost/asio.hpp>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <semaphore>

class AsyncServer;
class SocketUser;
class udp_main_server;

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::ip::address;
using boost::asio::ip::address_v4;

#ifndef MAX_UDP_SIZE
#define MAX_UDP_SIZE UINT16_MAX
#endif

class udp_connection {
	friend class udp_main_server;

	udp_main_server* m_udp_server;
	udp::socket m_socket_;
	address m_address;
	std::thread m_thread_sends;
	std::thread m_thread_service;
	uint8_t m_recv_buff[MAX_UDP_SIZE];
	uint8_t m_send_buff[MAX_UDP_SIZE];
	std::mutex m_send_lock;
	std::mutex m_lock;
	bool m_running;
	uint16_t m_port;
	udp::endpoint m_client_endpoint;
	udp::endpoint m_server_endpoint;

	int m_send_queue_len;
	int m_numTrySends;
	int m_numDoSends;

	struct Send_Message {
		std::vector<uint8_t> sending;
	};

	std::queue<Send_Message> m_send_messages;

	std::binary_semaphore m_sends_semaphore_1{ 0 };
	std::binary_semaphore m_sends_semaphore_2{ 0 };



public:
	typedef boost::shared_ptr<udp_connection> pointer;

	void Close();

	int GetSendQeueLength() {
		return m_send_queue_len;
	}

	int GetNumSendRequests() {
		return m_numTrySends;
	}

	int GetNumSends() {
		return m_numDoSends;
	}

	void Send(udp::endpoint remote_endpoint, uint8_t* sending, size_t len);

	void Send(udp::endpoint remote_endpoint, std::vector<uint8_t> sending);

	uint16_t GetPort() {
		return m_port;
	}

private:
	udp_connection(udp_main_server* udp_server, boost::asio::io_service& io_service, address addr, int port)
		: m_socket_(io_service, udp::endpoint(udp::v4(), port))
	{
		m_udp_server = udp_server;
		m_port = port;
		m_address = addr;
		m_running = true;
		m_send_queue_len = 0;
		m_numTrySends = 0;
		m_numDoSends = 0;
		m_server_endpoint = udp::endpoint(addr, m_port);
		m_client_endpoint = udp::endpoint(addr, m_port + 1);

		start_receive();

		m_thread_sends = std::thread(RunSend, this);
		m_thread_service = std::thread(RunService, this);
	}

	static void RunSend(udp_connection* srv) {
		while (srv->m_running) {
			//Logger::Log("Run send");
			srv->start_send();
			srv->m_sends_semaphore_1.acquire();
		}
	}

	void start_send();

	void handle_send();


	void start_receive();

	void handle_receive(const boost::system::error_code& error, size_t transfered);

	static void RunService(udp_connection* svr);

};
