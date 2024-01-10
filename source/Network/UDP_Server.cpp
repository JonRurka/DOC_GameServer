#include "UDP_Server.h"
#include "../Logger.h"
#include "AsyncServer.h"
#include "SocketUser.h"
#include "../Server_Main.h"

void udp_server::start_receive(std::shared_ptr<SocketUser> socket_user)
{
	int port = m_port;
	if (SERVER_LOCAL) {
		port += 1;
	}

	uint8_t* buffer = new uint8_t[MAX_UDP_SIZE];
	//ZeroMemory(buffer, MAX_UDP_SIZE);

	//socket_user->UdpEndPoint.port()
	udp::endpoint remote_endpoint = udp::endpoint(address_v4::any(), socket_user->UdpEndPoint.port()); //udp::endpoint(address_v4::any(), port);

	//Logger::Log("Receiving UDP from socket_user: " + std::to_string(remote_endpoint.port()));

	recv_socket_.async_receive_from(
		boost::asio::buffer(buffer, MAX_UDP_SIZE), remote_endpoint,
		boost::bind(&udp_server::handle_receive, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, buffer, remote_endpoint, socket_user));
}

void udp_server::close()
{
	Logger::Log("close UDP service.");
	m_running = false;
	io_service_.stop();
	m_thread.join();
}

void udp_server::handle_receive(const boost::system::error_code& error, size_t transfered, uint8_t* buffer, udp::endpoint endpoint, std::shared_ptr<SocketUser> socket_user)
{
	//start_receive(socket_user);
	return;


	if (error)
	{
		if (error.value() == boost::asio::error::operation_aborted) {
			/*Logger::Log("UDP receive operation aborted");
			if (socket_user->UdpEnabled) {
				Logger::Log("Reboot receive.");
				start_receive(socket_user);
				return;
			}
			else {
				Logger::Log("Not restarting listen for disconnected client.");
				return;
			}*/
		}

		Logger::Log("UDP Receive Error (" + std::to_string(error.value()) + "): " + error.what());
		start_receive(socket_user);
		return;

	}

	if (transfered <= 0) {
		Logger::Log("UDP Receive Buffer empty!");
		start_receive(socket_user);
		return;
	}

	//Logger::Log("received UDP packet");

	uint16_t size = *((uint16_t*)buffer);

	std::vector<uint8_t> msg(buffer + 2, buffer + (size + 2));

	delete[] buffer;

	//Logger::Log("Received buffer of length: " + std::to_string(msg.size()));

	//msg = BufferUtils::RemoveFront(Remove_LENGTH, msg);

	// Start receiving new data as soon as possible and yeet this data onto a queue.
	start_receive(socket_user);


	//async_server->Receive_UDP(msg, endpoint.address());

	Server_Main::SetMemoryUsageForThread("udp_service");
}

void udp_server::Send(udp::endpoint remote_endpoint, uint8_t* sending, size_t len)
{
	std::vector<uint8_t> msg(sending, sending + len);
	Send(remote_endpoint, msg);

}

void udp_server::Send(udp::endpoint remote_endpoint, std::vector<uint8_t> sending)
{
	sending = BufferUtils::AddLength(sending);

	uint8_t* buffer = new uint8_t[sending.size()];
	memcpy(buffer, sending.data(), sending.size());

	//Logger::Log("UDP Sent: " + std::to_string(sending.size()) + " on port " + std::to_string(remote_endpoint.port()));

	recv_socket_.async_send_to(boost::asio::buffer(buffer, sending.size()), remote_endpoint,
		boost::bind(&udp_server::handle_send, this, buffer));
}

void udp_server::AbortListen()
{
	Logger::Log("UDP aborting listens");
	recv_socket_.cancel(); // cancel events on SocketUser disconnect, and start listening again.
}

void udp_server::handle_send(uint8_t* buffer)
{
	delete[] buffer;

	//Logger::Log("Delete resources: " + std::to_string(send_buffers.size()));

	Server_Main::SetMemoryUsageForThread("udp_service");
}

void udp_server::RunService(udp_server* svr)
{
	
	Logger::Log("Running UPD io_service");
	while (svr->m_running) {
		svr->io_service_.run();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	Logger::Log("UDP io_service stopped running.");
	
}

