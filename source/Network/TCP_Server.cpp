#include "TCP_Server.h"
#include "AsyncServer.h"
#include "../Logger.h"

void tcp_server::start_accept()
{
	tcp_connection::pointer new_connection =
		tcp_connection::create(io_service_);

	Logger::Log("Waiting for connection....");

	acceptor_.async_accept(new_connection->socket(),
		boost::bind(&tcp_server::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
{
	start_accept();

	if (!error)
	{
		Logger::Log("Accepting new connection.");
		AsyncServer::SocketUser* user = new AsyncServer::SocketUser(
			async_server, 
			new_connection);
		char result = 0x00;
		user->HandleStartConnect();
		//new_connection->Send("test", 4);
	}
	else {
		Logger::Log("New connection failed.");
	}

	
}

void tcp_server::close()
{
	io_service_.stop();
	m_thread.join();
}
