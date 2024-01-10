#pragma once

#include "../stdafx.h"
#include "../Logger.h"

class AsyncServer;
class SocketUser;

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::ip::address;
using boost::asio::ip::address_v4;

#define MAX_UDP_SIZE UINT16_MAX

//socket_(io_service, udp::endpoint(udp::v4(), port)

class udp_server {
public:
	udp_server(AsyncServer* server, boost::asio::io_service& io_service, int port)
		: io_service_(io_service), 
		  //send_socket_(io_service, udp::v4()), 
		  recv_socket_(io_service, udp::endpoint(udp::v4(), port))
	{
		//recv_socket_(io_service, udp::endpoint(udp::v4(), port);
		//socket_.open(udp::v4());
		Logger::Log("Receiving UDP on port " + std::to_string(port));
		async_server = server;
		m_port = port;
		m_thread = std::thread(RunService, this);
	}

	void close();

	void Send(udp::endpoint remote_endpoint, uint8_t* sending, size_t len);

	void Send(udp::endpoint remote_endpoint, std::vector<uint8_t> sending);

	void start_receive(SocketUser* socket_user);

	void AbortListen();

private:
	
	void handle_receive(const boost::system::error_code& error, size_t transfered, uint8_t* buffer, udp::endpoint endpoint, SocketUser* socket_user);

	void handle_send(uint8_t* buffer);

	static void RunService(udp_server* svr);

	int m_port;
	//udp::socket send_socket_;
	udp::socket recv_socket_;
	AsyncServer* async_server;
	std::map<uint64_t, std::unique_ptr<uint8_t>> send_buffers;
	int numSends = 0;
	uint8_t recv_buffer_[MAX_UDP_SIZE];
	uint8_t length_buff[2];
	std::thread m_thread;
	boost::asio::io_service& io_service_;
	std::mutex m_send_lock;
	bool m_running;
};