#pragma once
#include "../stdafx.h"
#include "TCP_Connection.h"
#include <thread>

using boost::asio::ip::tcp;
using boost::asio::ip::address;

class AsyncServer;

class tcp_server {
private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	AsyncServer* async_server;
	std::thread m_thread;
	int m_port;

public:

	tcp_server(AsyncServer* server_inst, boost::asio::io_service& io_service, int port)
		: io_service_(io_service),
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		m_port = port;
		async_server = server_inst;
		start_accept();
		m_thread = std::thread(RunService, this);
	}

	void start_accept();

	void handle_accept(tcp_connection::pointer new_connection,
		const boost::system::error_code& error);

	void close();

private:
	static void RunService(tcp_server* svr) {
		svr->io_service_.run();
	}
};

