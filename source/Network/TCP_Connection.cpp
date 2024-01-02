#include "TCP_Connection.h"
#include "AsyncServer.h"
#include "../Logger.h"

void tcp_connection::Send(uint8_t* sending, size_t len)
{
	Send(std::vector(sending, sending + len));


	/*boost::shared_ptr<std::vector<char>> message(
		new std::vector(sending, sending + len));


	boost::asio::async_write(socket_, boost::asio::buffer(*message, len),
		boost::bind(&tcp_connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));*/
}

void tcp_connection::Send(std::vector<uint8_t> sending)
{
	sending = BufferUtils::AddLength(sending);

	//boost::shared_ptr<std::vector<uint8_t>> message(new std::vector(sending));
	uint64_t id = numSends++;
	send_buffers[id] = new uint8_t[sending.size()];
	memcpy(send_buffers[id], sending.data(), sending.size());

	boost::asio::async_write(socket_, boost::asio::buffer(send_buffers[id], sending.size()),
		boost::bind(&tcp_connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred, 
			id));
}

void tcp_connection::Start_Read()
{
	ZeroMemory(length_buff, 2);
	boost::asio::async_read(socket_, boost::asio::buffer(length_buff, 2),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void tcp_connection::Start_Initial_Connect()
{
	boost::asio::async_read(socket_, boost::asio::buffer(length_buff, 2),
		boost::bind(&tcp_connection::Handle_Initial_Connect, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void tcp_connection::close()
{
	socket_.close();
}

void tcp_connection::handle_write(const boost::system::error_code&, size_t transfered, uint64_t s_id)
{
	delete[] send_buffers[s_id];
	send_buffers.erase(s_id);
}

void tcp_connection::Handle_Initial_Connect(const boost::system::error_code& err, size_t transfered)
{

	if (err) {
		return;
	}

	uint16_t size = *((uint16_t*)&length_buff);

	uint8_t* message = new uint8_t[size];
	boost::asio::read(socket_, boost::asio::buffer(message, size));

	bool successfull = (size == 3 && message[0] == 0xff && message[1] == 0x01 && message[2] == 0x01);

	Logger::Log("Handle_Initial_Connect: " + std::to_string(successfull));
	if (!successfull) {
		Logger::Log("size: " + std::to_string(size));
		Logger::Log("message[0]: " + std::to_string(message[0]));
		Logger::Log("message[1]: " + std::to_string(message[1]));
		Logger::Log("message[2]: " + std::to_string(message[2]));
	}

	delete[] message;
	
	AsyncServer::SocketUser* socket_usr = (AsyncServer::SocketUser*)socket_user;
	socket_usr->HandleStartConnect_Finished(successfull);
}

void tcp_connection::handle_read(const boost::system::error_code& err, size_t transfered)
{
	AsyncServer::SocketUser* socket_usr = (AsyncServer::SocketUser*)socket_user;

	if (err) {
		Logger::Log("Error!");
		return;
	}

	uint16_t size = *((uint16_t*)&length_buff);

	if (size == 0) {
		Logger::Log("Buffer empty!");
		AsyncServer::GetInstance()->RemovePlayer(socket_usr);
		return;
	}

	uint8_t* message = new uint8_t[size];

	if (!socket_.is_open()) {
		Logger::Log("Socket closed!");
		AsyncServer::GetInstance()->RemovePlayer(socket_usr);
		return;
	}

	//Logger::Log("Received bytes: " + std::to_string(size));
	try {
		boost::asio::read(socket_, boost::asio::buffer(message, size));
	}
	catch (boost::system::system_error ex) {
		AsyncServer::GetInstance()->RemovePlayer(socket_usr);
		return;
	}

	// Start reading again only after we have all the data from the previous send.
	Start_Read();

	std::vector msg(message, message + size);
	delete[] message;

	
	socket_usr->ProcessReceiveBuffer(msg, Protocal_Tcp);
}