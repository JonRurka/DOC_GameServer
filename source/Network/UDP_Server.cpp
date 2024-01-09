#include "UDP_Server.h"
#include "../Logger.h"
#include "AsyncServer.h"
#include "SocketUser.h"

void udp_server::start_receive(std::shared_ptr<SocketUser> socket_user)
{
	int port = m_port;
	if (SERVER_LOCAL) {
		port += 1;
	}

	uint8_t* buffer = new uint8_t[MAX_UDP_SIZE];

	//socket_user->UdpEndPoint.port()
	udp::endpoint remote_endpoint = udp::endpoint(address_v4::any(), 11040); //udp::endpoint(address_v4::any(), port);

	Logger::Log("Receiving UDP from socket_user: " + std::to_string(remote_endpoint.port()));

	recv_socket_.async_receive_from(
		boost::asio::buffer(recv_buffer_, MAX_UDP_SIZE), remote_endpoint,
		boost::bind(&udp_server::handle_receive, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred/*, buffer, remote_endpoint, socket_user */ ));
}

void udp_server::close()
{
	Logger::Log("close UDP service.");
	io_service_.stop();
	m_thread.join();
}

void udp_server::handle_receive(const boost::system::error_code& error, size_t transfered/*, uint8_t* buffer, udp::endpoint endpoint, std::shared_ptr<SocketUser> socket_user*/ )
{
	uint8_t* buffer; 
	udp::endpoint endpoint; 
	std::shared_ptr<SocketUser> socket_user;

	Logger::Log("UDP error code: " + std::to_string(error.value()));


	if (error)
	{
		if (error.value() == boost::asio::error::operation_aborted) {
			Logger::Log("UDP receive operation aborted");
			if (socket_user->UdpEnabled) {
				Logger::Log("Reboot receive.");
				start_receive(socket_user);
				return;
			}
			else {
				Logger::Log("Not restarting listen for disconnected client.");
				return;
			}
		}

		Logger::Log("UDP Receive Error: " + error.what());
		start_receive(socket_user);
		return;

	}

	start_receive(socket_user);
	return;

	if (transfered <= 0) {
		Logger::Log("UDP Receive Buffer empty!");
		start_receive(socket_user);
		return;
	}

	Logger::Log("received UDP packet");

	//uint8_t* message = new uint8_t[transfered];
	//memcpy(message, recv_buffer_.data(), transfered);

	//uint16_t size = *((uint16_t*)&length_buff);

	//uint8_t* message = new uint8_t[size];

	/*try {
		//boost::asio::read(socket_, boost::asio::buffer(message, size));
	}
	catch (boost::system::system_error ex) {
		Logger::Log("UDP system_error!");
		start_receive();
		return;
	}*/

	//std::vector<uint8_t> msg(message, message + transfered);

	uint16_t size = *((uint16_t*)&buffer);
	std::vector<uint8_t> msg(buffer, buffer + (size + 2));

	delete buffer;

	//Logger::Log("Received buffer of length: " + std::to_string(msg.size()));

	msg = BufferUtils::RemoveFront(Remove_LENGTH, msg);

	// Start receiving new data as soon as possible and yeet this data onto a queue.
	start_receive(socket_user);

	//std::vector msg(message, message + transfered);
	//delete[] message;

	
	udp::endpoint remote_endpoint = udp::endpoint(address_v4::any(), m_port);

	async_server->Receive_UDP(msg, remote_endpoint.address());

}

void udp_server::Send(udp::endpoint remote_endpoint, uint8_t* sending, size_t len)
{
	std::vector<uint8_t> msg(sending, sending + len);
	Send(remote_endpoint, msg);

	/*boost::shared_ptr<std::vector<uint8_t>> message(
		new std::vector(sending, sending + len));

	socket_.async_send_to(boost::asio::buffer(*message, len), remote_endpoint_,
		boost::bind(&udp_server::handle_send, this));*/

}

void udp_server::Send(udp::endpoint remote_endpoint, std::vector<uint8_t> sending)
{
	sending = BufferUtils::AddLength(sending);

	//boost::shared_ptr<std::vector<uint8_t>> message(new std::vector(sending));

	m_send_lock.lock();
	uint64_t id = numSends++;
	send_buffers[id] = new uint8_t[sending.size()];
	m_send_lock.unlock();

	memcpy(send_buffers[id], sending.data(), sending.size());

	//Logger::Log("UDP Sent: " + std::to_string(sending.size()) + " on port " + std::to_string(remote_endpoint.port()));

	//recv_socket_.bind(remote_endpoint);

	recv_socket_.async_send_to(boost::asio::buffer(send_buffers[id], sending.size()), remote_endpoint,
		boost::bind(&udp_server::handle_send, this, id));
}

void udp_server::AbortListen()
{
	Logger::Log("UDP aborting listens");
	//recv_socket_.cancel(); // cancel events on SocketUser disconnect, and start listening again.
}

void udp_server::handle_send(uint64_t s_id)
{
	m_send_lock.lock();
	delete[] send_buffers[s_id];
	send_buffers.erase(s_id);
	m_send_lock.unlock();
}

