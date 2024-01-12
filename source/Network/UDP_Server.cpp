#include "UDP_Server.h"
#include "../Logger.h"
#include "AsyncServer.h"
#include "SocketUser.h"
#include "../Server_Main.h"
#include "../HashHelper.h"

udp_main_server* udp_main_server::m_instance = nullptr;

/*
void udp_server::start_receive(SocketUser* socket_user)
{
	int port = m_port;
	if (SERVER_LOCAL) {
		port += 1;
	}

	//socket_user->UdpEndPoint.port()
	udp::endpoint remote_endpoint = udp::endpoint(address_v4::any(), port);//udp::endpoint(address_v4::any(), socket_user->UdpEndPoint.port()); //udp::endpoint(address_v4::any(), port);

	//Logger::Log("Receiving UDP from socket_user: " + std::to_string(remote_endpoint.port()));

	m_lock.lock();
	recv_socket_.async_receive_from(
		boost::asio::buffer(recv_buffer_, MAX_UDP_SIZE), remote_endpoint,
		boost::bind(&udp_server::handle_receive, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, remote_endpoint, socket_user));
	m_lock.unlock();
}

void udp_server::close()
{
	Logger::Log("close UDP service.");
	m_running = false;
	io_service_.stop();
	m_thread_service.join();
	m_thread_sends.join();
}

void udp_server::handle_receive(const boost::system::error_code& error, size_t transfered, udp::endpoint endpoint, SocketUser* socket_user)
{

	if (error)
	{
		if (error.value() == 10061) { // refused by client
			start_receive(socket_user);
			// Possibly remove client?
			return;
		}

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

	uint16_t size = *((uint16_t*)recv_buffer_);

	//std::vector<uint8_t> msg(buffer + 2, buffer + (size + 2));


	//msg = BufferUtils::RemoveFront(Remove_LENGTH, msg);

	// Start receiving new data as soon as possible and yeet this data onto a queue.

	async_server->Receive_UDP(&recv_buffer_[2], size, endpoint.address());


	start_receive(socket_user);
	//async_server->Receive_UDP(msg, endpoint.address());

	//Server_Main::SetMemoryUsageForThread("udp_service");
}

void udp_server::Send(udp::endpoint remote_endpoint, uint8_t* sending, size_t len)
{
	std::vector<uint8_t> msg(sending, sending + len);
	Send(remote_endpoint, msg);

}

void udp_server::Send(udp::endpoint remote_endpoint, std::vector<uint8_t> sending)
{
	sending = BufferUtils::AddLength(sending);

	//Logger::Log("Send UDP message 1 ("+ std::to_string(sending.size()) + "): " + remote_endpoint.address().to_string() + ": " + std::to_string(remote_endpoint.port()));

	Send_Message msg{};
	msg.sending = sending;
	msg.remote_endpoint = remote_endpoint;

	m_send_lock.lock();
	m_send_messages.push(msg);
	m_send_queue_len++;
	m_numTrySends++;
	Server_Main::SetQueueLength_UDP_SendQeue(m_send_queue_len);
	m_send_lock.unlock();

	//Logger::Log("Push UDP message");
	m_sends_semaphore_2.release();
	
}

void udp_server::start_send()
{
	if (!m_running) {
		return;
	}

	m_sends_semaphore_2.acquire();

	m_send_lock.lock();
	Send_Message msg{};
	if (!m_send_messages.empty()) {
		msg = m_send_messages.front();
		m_send_messages.pop();

		m_send_queue_len--;
		Server_Main::SetQueueLength_UDP_SendQeue(m_send_queue_len);
	}
	else {
		m_send_lock.unlock();
		return;
	}
	m_send_lock.unlock();

	udp::endpoint remote_endpoint = msg.remote_endpoint;

	memcpy(m_send_buff, msg.sending.data(), msg.sending.size());

	recv_socket_.async_send_to(boost::asio::buffer(m_send_buff, msg.sending.size()), remote_endpoint,
		boost::bind(&udp_server::handle_send, this));

	m_numDoSends++;
	//Logger::Log("Send UDP message 2 (" + std::to_string(msg.sending.size()) + "): " + remote_endpoint.address().to_string() + ": " + std::to_string(remote_endpoint.port()));
	
}

void udp_server::handle_send()
{
	m_sends_semaphore_1.release();


	//Server_Main::SetMemoryUsageForThread("udp_service");
}

void udp_server::AbortListen()
{
	Logger::Log("UDP aborting listens");
	recv_socket_.cancel(); // cancel events on SocketUser disconnect, and start listening again.
}

void udp_server::RunService(udp_server* svr)
{
	
	Logger::Log("Running UPD io_service");
	svr->start_send();
	while (svr->m_running) {
		svr->io_service_.run();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	Logger::Log("UDP io_service stopped running.");
	
}
*/
// ############ NEW CODE ########################

void udp_main_server::close()
{
	Logger::Log("close UDP service.");
	m_run = false;
	io_service_.stop();
	m_thread.join();
}

udp_connection::pointer udp_main_server::create(address addr)
{
	int con_port = Get_New_Port();

	return udp_connection::pointer(new udp_connection(this, io_service_, addr, con_port));
}

void udp_main_server::RunService(udp_main_server* svr)
{
	Logger::Log("Running UPD io_service");
	while (svr->m_run) {
		svr->io_service_.run();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	Logger::Log("UDP io_service stopped running.");

}

uint16_t udp_main_server::Get_New_Port()
{
	uint16_t newNum = HashHelper::RandomNumber(m_port_range_start, m_port_range_end);
	if (has_port(newNum)) {
		newNum = Get_New_Port();
	}
	return newNum;
}
