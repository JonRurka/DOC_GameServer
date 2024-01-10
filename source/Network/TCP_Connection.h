#pragma once
#include "../stdafx.h"

class AsyncServer;
class SocketUser;

using boost::asio::ip::tcp;
using boost::asio::ip::address;

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
	
private:
	tcp::socket socket_;
	//std::map<int, std::vector<char>> send_buff;
	int sent;
	uint8_t length_buff[2]{};
	std::map<uint64_t, uint8_t*> send_buffers;
	int numSends = 0;
	std::weak_ptr<SocketUser> socket_user;
	std::vector<std::shared_ptr<SocketUser>> tmp_socket_ref;
public:
	typedef boost::shared_ptr<tcp_connection> pointer;

	static pointer create(boost::asio::io_service& io_service)
	{
		return pointer(new tcp_connection(io_service));
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void Set_Socket_User(std::weak_ptr<SocketUser> p_socket_user) {
		socket_user = p_socket_user;
	}

	void Send(uint8_t* sending, size_t len);

	void Send(std::vector<uint8_t> sending);

	void Start_Read();

	void Start_Initial_Connect(std::shared_ptr<SocketUser> p_socket_user);

	void close();

private:
	tcp_connection(boost::asio::io_service& io_service)
		: socket_(io_service)
	{
		sent = 0;
	}

	void handle_write(const boost::system::error_code&, size_t transfered, uint8_t* buffer);

	void Handle_Initial_Connect(
		const boost::system::error_code&, 
		size_t transfered,
		SocketUser* p_socket_user);

	void handle_read(const boost::system::error_code&, size_t transfered);
};