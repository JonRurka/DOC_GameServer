#include "udp_connection.h"
#include "UDP_Server.h"
#include "../Logger.h"
#include "AsyncServer.h"
#include "SocketUser.h"
#include "../Server_Main.h"

void udp_connection::Close()
{
	Logger::Log("close UDP service.");
	m_running = false;
	m_socket_.close();
	m_thread_sends.join();
	m_udp_server->m_connections.erase(m_port);
}

void udp_connection::Send(udp::endpoint remote_endpoint, uint8_t* sending, size_t len)
{
	std::vector<uint8_t> msg(sending, sending + len);
	Send(remote_endpoint, msg);
}

void udp_connection::Send(udp::endpoint remote_endpoint, std::vector<uint8_t> sending)
{
	sending = BufferUtils::AddLength(sending);

	//Logger::Log("Send UDP message 1 ("+ std::to_string(sending.size()) + "): " + remote_endpoint.address().to_string() + ": " + std::to_string(remote_endpoint.port()));

	Send_Message msg{};
	msg.sending = sending;
	//msg.remote_endpoint = remote_endpoint;

	m_send_lock.lock();
	m_send_messages.push(msg);
	m_send_queue_len++;
	m_numTrySends++;
	Server_Main::SetQueueLength_UDP_SendQeue(m_send_queue_len);
	m_send_lock.unlock();

	//Logger::Log("Push UDP message");
	m_sends_semaphore_2.release();
}

void udp_connection::start_send()
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

	memcpy(m_send_buff, msg.sending.data(), msg.sending.size());

	m_socket_.async_send_to(boost::asio::buffer(m_send_buff, msg.sending.size()), m_client_endpoint,
		boost::bind(&udp_connection::handle_send, this));

	m_numDoSends++;
}

void udp_connection::handle_send()
{
	m_sends_semaphore_1.release();
}


void udp_connection::start_receive()
{
	m_socket_.async_receive_from(
		boost::asio::buffer(m_recv_buff, MAX_UDP_SIZE), m_client_endpoint,
		boost::bind(&udp_connection::handle_receive, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void udp_connection::handle_receive(const boost::system::error_code& error, size_t transfered)
{
	if (error)
	{
		if (error.value() == 10061) { // refused by client
			start_receive();
			// Possibly remove client?
			return;
		}

		Logger::Log("UDP Receive Error (" + std::to_string(error.value()) + "): " + error.what());
		start_receive();
		return;
	}

	if (transfered <= 0) {
		Logger::Log("UDP Receive Buffer empty!");
		start_receive();
		return;
	}

	uint16_t size = *((uint16_t*)m_recv_buff);

	//uint8_t* tmp = &m_recv_buff[2];
	//for (int i = 0; i < size; i++) {
		//Logger::Log(std::to_string(i) + ": " + std::to_string(tmp[i]));
	//}

	//Logger::Log("Received UDP message");

	AsyncServer::GetInstance()->Receive_UDP(&m_recv_buff[2], size, m_client_endpoint.address());

	start_receive();

}
