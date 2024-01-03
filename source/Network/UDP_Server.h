#pragma once

#include "../stdafx.h"

class AsyncServer;

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::ip::address;
using boost::asio::ip::address_v4;

#define MAX_UDP_SIZE UINT16_MAX

class udp_server {
public:
	udp_server(AsyncServer* server, boost::asio::io_service& io_service, int port)
		: io_service_(io_service), socket_(io_service, udp::endpoint(udp::v4(), port))
	{
		async_server = server;
		m_port = port;
		start_receive();
		m_thread = std::thread(RunService, this);
	}

	void close();

	void Send(udp::endpoint remote_endpoint, uint8_t* sending, size_t len);

	void Send(udp::endpoint remote_endpoint, std::vector<uint8_t> sending);

private:
	void start_receive();

	void handle_receive(const boost::system::error_code& error, size_t transfered);

	void handle_send(uint64_t s_id);

	static void RunService(udp_server* svr) {
		svr->io_service_.run();
	}

	int m_port;
	udp::socket socket_;
	AsyncServer* async_server;
	std::map<uint64_t, uint8_t*> send_buffers;
	int numSends = 0;
	uint8_t recv_buffer_[MAX_UDP_SIZE];
	uint8_t length_buff[2];
	std::thread m_thread;
	boost::asio::io_service& io_service_;
};