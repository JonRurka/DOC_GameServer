#include "TCP_Connection.h"
#include "AsyncServer.h"
#include "SocketUser.h"
#include "../Logger.h"
#include "../Server_Main.h"

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
	//uint64_t id = numSends++;
	uint8_t* buffer = new uint8_t[sending.size()];
	//send_buffers[id] = new uint8_t[sending.size()];
	memcpy(buffer, sending.data(), sending.size());

	boost::asio::async_write(socket_, boost::asio::buffer(buffer, sending.size()),
		boost::bind(&tcp_connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred, 
			buffer));
}

void tcp_connection::Start_Read()
{
	ZeroMemory(length_buff, 2);
	boost::asio::async_read(socket_, boost::asio::buffer(length_buff, 2),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void tcp_connection::Start_Initial_Connect(std::shared_ptr<SocketUser> p_socket_user)
{
	Set_Socket_User(p_socket_user);
	tmp_socket_ref.push_back(p_socket_user);

	boost::asio::async_read(socket_, boost::asio::buffer(length_buff, 2),
		boost::bind(&tcp_connection::Handle_Initial_Connect, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			p_socket_user.get()));
}

void tcp_connection::close()
{
	socket_.close();
}

void tcp_connection::handle_write(const boost::system::error_code&, size_t transfered, uint8_t* buffer)
{
	delete[] buffer;
	//send_buffers.erase(s_id);

	Server_Main::SetMemoryUsageForThread("tcp_service");
}

void tcp_connection::Handle_Initial_Connect(
	const boost::system::error_code& err, 
	size_t transfered, 
	SocketUser* p_socket_user)
{
	Logger::Log("Handle_Initial_Connect");
	if (err) {
		Logger::Log("Initial Connection error: " + err.what());
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
	
	//SocketUser* socket_usr = (SocketUser*)socket_user;
	tmp_socket_ref[0]->HandleStartConnect_Finished(successfull);
	tmp_socket_ref.clear();

	Server_Main::SetMemoryUsageForThread("tcp_service");
}

void tcp_connection::handle_read(const boost::system::error_code& err, size_t transfered)
{
	//SocketUser* socket_usr = (SocketUser*)socket_user;

	if (err) {
		if (err.value() == 10054) {
			Logger::Log("TCP client disconnected.");
			socket_user.lock()->Close(false);
			return;
		}

		Logger::Log("TCP Read Error (" + std::to_string(err.value()) + "): " + err.what());
		return;
	}

	uint16_t size = *((uint16_t*)&length_buff);

	if (size == 0) {
		Logger::Log("TCP Buffer empty!");
		AsyncServer::GetInstance()->RemovePlayer(socket_user.lock());
		return;
	}

	uint8_t* message = new uint8_t[size];

	if (!socket_.is_open()) {
		Logger::Log("TCP Socket closed!");
		AsyncServer::GetInstance()->RemovePlayer(socket_user.lock());
		return;
	}

	//Logger::Log("Received bytes: " + std::to_string(size));
	try {
		boost::asio::read(socket_, boost::asio::buffer(message, size));
	}
	catch (boost::system::system_error ex) {
		AsyncServer::GetInstance()->RemovePlayer(socket_user.lock());
		return;
	}

	// Start reading again only after we have all the data from the previous send.
	Start_Read();

	std::vector msg(message, message + size);
	delete[] message;

	
	socket_user.lock()->ProcessReceiveBuffer(msg, Protocal_Tcp);

	Server_Main::SetMemoryUsageForThread("tcp_service");
}