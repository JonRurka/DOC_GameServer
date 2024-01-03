#include "UDP_Server.h"
#include "../Logger.h"
#include "AsyncServer.h"

void udp_server::start_receive()
{
	int port = m_port;
	if (SERVER_LOCAL) {
		port += 1;
	}

	udp::endpoint remote_endpoint = udp::endpoint(address_v4::any(), port);

	socket_.async_receive_from(
		boost::asio::buffer(recv_buffer_, MAX_UDP_SIZE), remote_endpoint,
		boost::bind(&udp_server::handle_receive, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void udp_server::close()
{
	io_service_.stop();
	m_thread.join();
}

void udp_server::handle_receive(const boost::system::error_code& error, size_t transfered)
{
	if (error)
	{
		Logger::Log("UDP Error!");
		start_receive();
		return;
	}

	if (transfered <= 0) {
		Logger::Log("UDP Buffer empty!");
		start_receive();
		return;
	}

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

	uint16_t size = *((uint16_t*)&recv_buffer_);
	std::vector<uint8_t> msg(recv_buffer_, recv_buffer_ + (size + 2));

	//Logger::Log("Received buffer of length: " + std::to_string(msg.size()));

	msg = BufferUtils::RemoveFront(Remove_LENGTH, msg);

	// Start receiving new data as soon as possible and yeet this data onto a queue.
	start_receive();

	//std::vector msg(message, message + transfered);
	//delete[] message;

	
	
	async_server->Receive_UDP(msg);

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
	uint64_t id = numSends++;
	send_buffers[id] = new uint8_t[sending.size()];
	memcpy(send_buffers[id], sending.data(), sending.size());

	Logger::Log("UDP Sent: " + std::to_string(sending.size()));

	socket_.async_send_to(boost::asio::buffer(send_buffers[id], sending.size()), remote_endpoint,
		boost::bind(&udp_server::handle_send, this, id));
}

void udp_server::handle_send(uint64_t s_id)
{
	delete[] send_buffers[s_id];
	send_buffers.erase(s_id);
}

