#include "AsyncServer.h"
#include "SocketUser.h"
#include "../Server_Main.h"
#include "../Logger.h"
#include "../HashHelper.h"
#include <thread>

AsyncServer* AsyncServer::m_instance = nullptr;

AsyncServer::AsyncServer(Server_Main* server)
{
    m_instance = this;

	m_server = server;
	//m_authenticator = PlayerAuthenticator(this);

    m_udp_server = new udp_server(this, m_io_service_udp, UDP_PORT);
	m_tcp_server = new tcp_server(this, m_io_service_tcp, TCP_PORT);

    AddCommand(OpCodes::Server::System_Reserved, System_Cmd_cb, this);

	Logger::Log("Net server started.");

    if (RUN_ASYNC_COMMANDS) {
        m_thread_1 = std::thread(Process_Async, this);
    }

    //m_thread_2 = std::thread(Test_Client);
}

void AsyncServer::Update(float dt)
{
    m_user_mtx.lock();
    for (const auto& user : m_socket_users) {
        // TODO: Fix crash
        user.second->Update(dt);
    }
    m_user_mtx.unlock();
    

    while (!m_main_command_queue.empty()) {
        ThreadCommand thr_command = m_main_command_queue.front();
        m_main_command_queue.pop();

        if (!thr_command.user.expired()) {
            DoProcess(thr_command.user.lock(), thr_command.data);
        }
    }
}

void AsyncServer::Process_Async(AsyncServer* svr) {

    svr->m_run = true;
    svr->m_run_async_commands = true;

    while (svr->m_run) {

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        while (!svr->m_async_command_queue.empty()) {
            ThreadCommand thr_command = svr->m_async_command_queue.front();
            svr->m_async_command_queue.pop();

            if (!thr_command.user.expired()) {
                svr->DoProcess(thr_command.user.lock(), thr_command.data);
            }
        }
    }
}

void AsyncServer::AddCommand(OpCodes::Server cmd, CommandActionPtr callback, void* obj, bool async)
{
    if (!HasCommand((uint8_t)cmd)) {
        NetCommand command;
        command.Callback = callback;
        command.Obj_Ptr = obj;
        command.Is_Async = async;
        m_commands[(uint8_t)cmd] = command;
    }
}

void AsyncServer::AddPlayer(std::shared_ptr<SocketUser> user)
{
    m_user_mtx.lock();
    if (!HasPlayerSession(user->SessionToken)) {
        m_socket_users[user->SessionToken] = user;
        m_server->UserConnected(user);
    }
    m_user_mtx.unlock();
}

void AsyncServer::RemovePlayer(std::shared_ptr<SocketUser> user)
{
    m_user_mtx.lock();
    std::string session_token = user->SessionToken;
    uint16_t udp_id =  user->Get_UDP_ID();
    if (HasPlayerSession(session_token)) {
        m_socket_users.erase(session_token);
        m_udp_id_map.erase(udp_id);
        m_server->UserDisconnected(user);
        user->Close(false);
        //delete user;
    }
    m_user_mtx.unlock();
    
}

bool AsyncServer::HasPlayerSession(std::string session_key)
{
    return m_socket_users.find(session_key) != m_socket_users.end();
}

void AsyncServer::PlayerAuthenticated(std::shared_ptr<SocketUser> user, bool authorized)
{
	if (authorized) {
		// set to authorized/

		m_server->UserConnected(user);

	}
	else {
		// Send rejected packet
		
		// Destroy user.
		//delete user;
	}
}



void AsyncServer::Test_Client()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    boost::asio::io_service io_service;
    //socket creation
    tcp::socket socket(io_service);
    //connection
    socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1000));
    // request/message from client
    const std::string msg = "Hello from Client!\n";
    boost::system::error_code error;
    boost::asio::write(socket, boost::asio::buffer(msg), error);
    if (!error) {
        Logger::Log("client sent hello");
        //cout << "Client sent hello message!" << endl;
    }
    else {
        Logger::Log("Failed to send hello from client");
        //cout << "send failed: " << error.message() << endl;
    }
    // getting response from server
    boost::asio::streambuf receive_buffer;
    boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
    if (error && error != boost::asio::error::eof) {
        Logger::Log("received a fail response");
        //cout << "receive failed: " << error.message() << endl;
    }
    else {
        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
        Logger::Log("received a successful response: " + std::string(data));
        //cout << data << endl;
    }

}

std::string read_(tcp::socket& socket) {
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");
    std::string data = boost::asio::buffer_cast<const char*>(buf.data());
    return data;
}
void send_(tcp::socket& socket, const std::string& message) {
    const std::string msg = message + "\n";
    boost::asio::write(socket, boost::asio::buffer(message));
}

void AsyncServer::Test_Server(void* obj)
{
    AsyncServer* async_s = (AsyncServer*)obj;

    boost::asio::io_service io_service;
    //listen for new connection
    tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 1234));
    //socket creation 
    tcp::socket socket_(io_service);
    //waiting for connection
    
    async_s->m_thread_2 = std::thread(Test_Client);

    //acceptor_.accept(socket_);
    acceptor_.async_accept(socket_,
        boost::bind(&AsyncServer::handle_accept, async_s,
            boost::asio::placeholders::error));

    io_service.run();
    

    //Logger::Log("Client connected");
    //read operation
    //std::string message = read_(socket_);
    //Logger::Log(message);
    //cout << message << endl;
    //write operation
    //send_(socket_, "Hello From Server!");
    //Logger::Log("Servent sent Hello message to Client!");
    //cout << "Servent sent Hello message to Client!" << endl;
}

void AsyncServer::Receive_UDP(std::vector<uint8_t> buffer, boost::asio::ip::address endpoint)
{
    if (buffer.size() >= 3)
    {
        uint16_t udp_id = *((uint16_t*)buffer.data());
        buffer = BufferUtils::RemoveFront(Remove_UDP_ID, buffer);

        uint8_t command = buffer[0];
        buffer = BufferUtils::RemoveFront(Remove_CMD, buffer);
        
        if (Has_UDP_ID(udp_id)) {

            //Logger::Log(endpoint.address().to_string());
            
            // Compare address to client.

            std::shared_ptr<SocketUser> socket_user = m_udp_id_map[udp_id];
            Data data(Protocal_Udp, command, buffer);
            Process(socket_user, data);
        }
        else {
            Logger::Log("UDP ID Not Found: " + std::to_string(udp_id));
        }
    }
    else {
        Logger::Log(std::to_string(Protocal_Udp) + ": Received empty buffer!");
    }
}

void AsyncServer::handle_accept(const boost::system::error_code& error) {
    if (!error)
        Logger::Log("Client connected");
    else
        Logger::Log("Client connect failed");
}

void AsyncServer::Process(std::shared_ptr<SocketUser> socket_user, Data data)
{
    if (HasCommand(data.command)) {

        NetCommand command_obj = m_commands[data.command];

        socket_user->ResetPingCounter(); 
        
        ThreadCommand thread_cmd;
        thread_cmd.data = data;
        thread_cmd.user = socket_user;

        if (command_obj.Is_Async && m_run_async_commands) {
            m_async_command_queue.push(thread_cmd);
        }
        else {
            m_main_command_queue.push(thread_cmd);
        }
    }
    else {
        Logger::Log("User submitted invalid command");
    }
}

uint16_t AsyncServer::Get_New_UDP_ID()
{
    uint16_t newNum = HashHelper::RandomNumber(0, UINT16_MAX);
    if (Has_UDP_ID(newNum)) {
        newNum = Get_New_UDP_ID();
    }
    return newNum;
}

void AsyncServer::DoProcess(std::shared_ptr<SocketUser> socket_user, Data data) {
    NetCommand command_obj = m_commands[data.command];
    command_obj.Callback(command_obj.Obj_Ptr, socket_user, data);
}

void AsyncServer::System_Cmd(std::shared_ptr<SocketUser> socket_user, Data data)
{
    uint8_t sub_command = data.Buffer[0];
    //UE_LOG(GameClient_Log, Display, TEXT("Received sub command %d"), sub_command);
    //Logger::Log("Sub Command: " + std::to_string(sub_command));

    data.Buffer = BufferUtils::RemoveFront(Remove_CMD, data.Buffer);

    switch (sub_command) {
    case 0x02: // disconnect
        RemovePlayer(socket_user);
        break;
    case 0x03: // ping
        socket_user->ResetPingCounter();
        socket_user->Send(OpCodes::Client::System_Reserved, std::vector<uint8_t>({ 0x03, 0x01 }), Protocal_Tcp);
        //Logger::Log("Received ping");
        break;
    case 0x04: // set UDP client port
        uint16_t port = *((uint16_t*)data.Buffer.data());
        socket_user->Set_Client_UDP_Port(port);
        break;
    }
}


