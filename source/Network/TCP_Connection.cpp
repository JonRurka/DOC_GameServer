#include "TCP_Connection.h"
#include "AsyncServer.h"
#include "SocketUser.h"
#include "../Logger.h"
#include "../Server_Main.h"

void tcp_connection::Send(uint8_t* sending, size_t len)
{
	Send(std::vector(sending, sending + len));

}

void tcp_connection::Send(std::vector<uint8_t> sending)
{
	sending = BufferUtils::AddLength(sending);

	//boost::shared_ptr<std::vector<uint8_t>> message(new std::vector(sending));
	//uint64_t id = numSends++;

	Send_Message msg{};
	msg.sending = sending;

	m_send_lock.lock();
	m_send_messages.push(msg);
	m_send_lock.unlock();

	m_sends_semaphore_2.release();
}

void tcp_connection::Start_Read()
{
	m_lock.lock();
	boost::asio::async_read(socket_, boost::asio::buffer(length_buff, 2),
		boost::bind(&tcp_connection::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	m_lock.unlock();
}

void tcp_connection::Start_Initial_Connect(std::shared_ptr<SocketUser> p_socket_user)
{
	Set_Socket_User(p_socket_user);
	tmp_socket_ref.push_back(p_socket_user);

	m_lock.lock();
	boost::asio::async_read(socket_, boost::asio::buffer(length_buff, 2),
		boost::bind(&tcp_connection::Handle_Initial_Connect, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			p_socket_user.get()));
	m_lock.unlock();
}

void tcp_connection::close()
{
	m_running = false;
	socket_.close();
	m_thread_sends.join();
}

void tcp_connection::start_send() {
	if (!m_running) {
		return;
	}

	m_sends_semaphore_2.acquire();

	m_send_lock.lock();
	Send_Message msg{};
	if (!m_send_messages.empty()) {
		msg = m_send_messages.front();
		m_send_messages.pop();
	}
	else {
		m_send_lock.unlock();
		return;
	}
	m_send_lock.unlock();

	memcpy(m_send_buffer, msg.sending.data(), msg.sending.size());

	//m_lock.lock();
	boost::asio::async_write(socket_, boost::asio::buffer(m_send_buffer, msg.sending.size()),
		boost::bind(&tcp_connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	//m_lock.unlock();
}

void tcp_connection::handle_write(const boost::system::error_code&, size_t transfered)
{

	m_sends_semaphore_1.release();

	//Server_Main::SetMemoryUsageForThread("tcp_service");
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
		AsyncServer::GetInstance()->RemovePlayer(socket_user.lock()->SessionToken);
		return;
	}

	uint8_t* message = new uint8_t[size];

	if (!socket_.is_open()) {
		Logger::Log("TCP Socket closed!");
		AsyncServer::GetInstance()->RemovePlayer(socket_user.lock()->SessionToken);
		return;
	}

	//Logger::Log("Received bytes: " + std::to_string(size));
	try {
		boost::asio::read(socket_, boost::asio::buffer(message, size));
	}
	catch (boost::system::system_error ex) {
		AsyncServer::GetInstance()->RemovePlayer(socket_user.lock()->SessionToken);
		return;
	}

	// Start reading again only after we have all the data from the previous send.

	std::vector msg(message, message + size);
	delete[] message;

	if (!socket_user.expired())
		socket_user.lock()->ProcessReceiveBuffer(msg, Protocal_Tcp);

	Start_Read();

	//Server_Main::SetMemoryUsageForThread("tcp_service");
}