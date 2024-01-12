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

    //m_udp_server = new udp_server(this, m_io_service_udp, UDP_PORT);
	m_tcp_server = new tcp_server(this, m_io_service_tcp, TCP_PORT);
    m_udp_server = new udp_main_server(this, m_io_service_udp, UDP_PORT_RANGE_START, UDP_PORT_RANGE_END);

    AddCommand(OpCodes::Server::System_Reserved, System_Cmd_cb, this);

	Logger::Log("Net server started.");

    if (RUN_ASYNC_COMMANDS) {
        m_thread_1 = std::thread(Process_Async, this);
    }

    //m_thread_2 = std::thread(Test_Client);
}

void AsyncServer::Update(float dt)
{
    while (!m_queue_user_add.empty()) {
        std::shared_ptr<SocketUser>& user = m_queue_user_add.front();
        m_queue_user_add.pop();

        DoAddPlayer(user);
    }

    while (!m_queue_user_remove.empty()) {
        std::string user = m_queue_user_remove.front();
        m_queue_user_remove.pop();

        DoRemovePlayer(user);
    }

    std::string use_count_str;
    for (auto& usr : m_Socket_user_list) {
        usr->Update(dt);

        use_count_str += std::to_string(usr.use_count()) + ", ";
    }

    Server_Main::SetQueueLength_TCP_SendQeue(Get_TCP_Send_Queue_Size_All());
    Server_Main::SetQueueLength_UDP_SendQeue(Get_UDP_Send_Queue_Size_All());

    //if (m_Socket_user_list.size() > 0)
    //    Logger::Log("User use counts: " + use_count_str);

    std::queue<ThreadCommand> current_command_queue;
    int numEntries = threadSafeCommandQueueDuplicate(m_main_command_queue_lock, m_main_command_queue, current_command_queue);

    m_main_cmd_q_len = numEntries;
    Server_Main::SetQueueLength_Main_ReceiveQueue(numEntries);

    while (!current_command_queue.empty()) {
        ThreadCommand thr_command = current_command_queue.front();
        current_command_queue.pop();
        Data data(thr_command.type, thr_command.command, std::vector<uint8_t>(thr_command.buffer, thr_command.buffer + thr_command.buffer_size));
        delete[] (thr_command.buffer);
        DoProcess(thr_command.user, data);
    }
}

void AsyncServer::Process_Async(AsyncServer* svr) {

    svr->m_run = true;
    svr->m_run_async_commands = true;

    uint64_t last_command_processed = 0;
    int ns_wait = 1000;

    while (svr->m_run) {

        std::this_thread::sleep_for(std::chrono::nanoseconds(ns_wait));

        std::queue<ThreadCommand> current_command_queue;
        int numEntries = svr->threadSafeCommandQueueDuplicate(svr->m_async_command_queue_lock, svr->m_async_command_queue, current_command_queue);

        svr->m_async_cmd_q_len = numEntries;
        Server_Main::SetQueueLength_Async_ReceiveQueue(numEntries);

        if (numEntries == 0) {
            continue;
        }

        uint64_t now = Server_Main::GetEpoch();
        uint64_t time_diff_ms = (now - last_command_processed);
        last_command_processed = now;

        if (time_diff_ms > 10000) { // no command in 10 seconds
            ns_wait = 1000; // 1 ms
        }
        else if (time_diff_ms > 1000) { // no command in 1 second
            ns_wait = 100;
        }
        else {
            ns_wait = 1;
        }

        while (!current_command_queue.empty()) {
            ThreadCommand thr_command = current_command_queue.front();
            current_command_queue.pop();
            Data data(thr_command.type, thr_command.command, std::vector<uint8_t>(thr_command.buffer, thr_command.buffer + thr_command.buffer_size));
            delete[](thr_command.buffer);
            svr->DoProcess(thr_command.user, data);
        }

        //Server_Main::SetMemoryUsageForThread("async_server_process");
    }
}

int AsyncServer::threadSafeCommandQueueDuplicate(std::mutex& lock, std::queue<ThreadCommand>& from, std::queue<ThreadCommand>& to)
{
    int res = 0;
    lock.lock();
    while (!from.empty()) {
        to.push(from.front());
        from.pop();
        res++;
    }
    lock.unlock();
    return res;
}

void AsyncServer::AddCommand(OpCodes::Server cmd, CommandActionPtr callback, void* obj, bool async)
{
    if (!HasCommand((uint8_t)cmd)) {
        NetCommand command{};
        command.Callback = callback;
        command.Obj_Ptr = obj;
        command.Is_Async = async;
        m_commands[(uint8_t)cmd] = command;
    }
}

void AsyncServer::AddPlayer(std::shared_ptr<SocketUser> user)
{
    m_queue_user_add.push(user);
}

void AsyncServer::RemovePlayer(std::string user)
{
    m_queue_user_remove.push(user);
}

void AsyncServer::DoAddPlayer(std::shared_ptr<SocketUser> user) {
    if (!HasPlayerSession(user->SessionToken)) {
        m_user_mtx.lock();
        m_socket_users[user->SessionToken] = user;
        m_server->UserConnected(user);
        m_user_mtx.unlock();
        PopulateUserList();
    }
}

void AsyncServer::DoRemovePlayer(std::string user) {
    std::string session_token = user;
    if (HasPlayerSession(session_token)) {

        SocketUser* user_obj = m_socket_users[session_token].get();
        uint16_t udp_id = user_obj->Get_UDP_ID();
        m_server->UserDisconnected(user_obj);
        user_obj->Close(false);

        m_user_mtx.lock();
        m_socket_users.erase(session_token);
        m_udp_id_map.erase(udp_id);
        m_user_mtx.unlock();

        PopulateUserList();
        //delete user;
    }
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

int AsyncServer::Get_TCP_Send_Queue_Size_All()
{
    if (m_Socket_user_list.size() == 0)
        return 0;

    int res = 0;
    for (auto& usr : m_Socket_user_list) {
        res += usr->tcp_connection_client->GetSendQeueLength();
    }
    return res / m_Socket_user_list.size();
}

int AsyncServer::Get_UDP_Send_Queue_Size_All()
{
    if (m_Socket_user_list.size() == 0)
        return 0;

    int res = 0;
    for (auto& usr : m_Socket_user_list) {
        res += usr->udp_connection_client->GetSendQeueLength();
    }
    return res / m_Socket_user_list.size();
}

glm::uvec2 AsyncServer::Get_UDP_Sends()
{
    return glm::uvec2();

    //return glm::uvec2(m_udp_server->GetNumSendRequests(), m_udp_server->GetNumSends());
}

void AsyncServer::Receive_UDP(std::vector<uint8_t> buffer, boost::asio::ip::address endpoint)
{
    if (buffer.size() >= 3)
    {
        if (buffer.size() > MAX_UDP_SIZE) {
            Logger::LogWarning("Received malformed UDP packet");
            //return;
        }

        uint16_t udp_id = *((uint16_t*)buffer.data());
        buffer = BufferUtils::RemoveFront(Remove_UDP_ID, buffer);

        uint8_t command = buffer[0];
        buffer = BufferUtils::RemoveFront(Remove_CMD, buffer);
        
        if (Has_UDP_ID(udp_id)) {

            //Logger::Log(endpoint.address().to_string());
            
            // Compare address to client.

            SocketUser* socket_user = m_udp_id_map[udp_id].get();
            Data data(Protocal_Udp, command, buffer);
            //Process(socket_user, data);
        }
        else {
            Logger::Log("UDP ID Not Found: " + std::to_string(udp_id));
        }
    }
    else {
        Logger::Log(std::to_string(Protocal_Udp) + ": Received empty buffer!");
    }
}

void AsyncServer::Receive_UDP(uint8_t* data, uint16_t size, boost::asio::ip::address endpoint)
{
    if (size >= 3)
    {
        uint16_t udp_id = *((uint16_t*)data);
        uint8_t* buffer = &data[2];
        //uint8_t* buffer = BufferUtils::RemoveFront(Remove_UDP_ID, buffer);

        uint8_t command = buffer[0];
        buffer = &buffer[1];
        //buffer = BufferUtils::RemoveFront(Remove_CMD, buffer);

        if (Has_UDP_ID(udp_id)) {

            //Logger::Log(endpoint.address().to_string());

            // Compare address to client.

            SocketUser* socket_user = m_udp_id_map[udp_id].get();
            //Data data(Protocal_Udp, command, buffer);
            Process(socket_user, command, buffer, size, Protocal_Udp);
        }
        else {
            Logger::Log("UDP ID Not Found: " + std::to_string(udp_id));
        }
    }
    else {
        Logger::Log(std::to_string(Protocal_Udp) + ": Received empty buffer!");
    }
    assert(size <= MAX_UDP_SIZE);
}

void AsyncServer::handle_accept(const boost::system::error_code& error) {
    if (!error)
        Logger::Log("Client connected");
    else
        Logger::Log("Client connect failed");
}

/*void AsyncServer::Process(SocketUser* socket_user, Data data)
{
    if (HasCommand(data.command)) {

        NetCommand command_obj = m_commands[data.command];

        socket_user->ResetPingCounter(); 
        
        ThreadCommand thread_cmd;
        thread_cmd.data = data;
        thread_cmd.user = socket_user->SessionToken;
        
        if (command_obj.Is_Async && m_run_async_commands) {
            //Logger::Log("Received async command: " + std::to_string(data.command));
            m_async_command_queue.push(thread_cmd);
        }
        else {
            //Logger::Log("Received main command: " + std::to_string(data.command));
            m_main_command_queue.push(thread_cmd);
        }
    }
    else {
        Logger::Log("User submitted invalid command");
    }
}*/

void AsyncServer::Process(SocketUser* socket_user, uint8_t command, uint8_t* data, int size, Protocal type)
{
    //if (HasCommand(command)) {

        NetCommand command_obj = m_commands[command];

        socket_user->ResetPingCounter();

        ThreadCommand thread_cmd{};
        thread_cmd.buffer = new uint8_t[size];
        memcpy(thread_cmd.buffer, data, size);
        thread_cmd.buffer_size = size;
        thread_cmd.type = type;
        thread_cmd.user = socket_user->SessionToken;
        thread_cmd.command = command;

        if (command_obj.Is_Async && m_run_async_commands) {
            //Logger::Log("Received async command: " + std::to_string(data.command));
            m_async_command_queue_lock.lock();
            m_async_command_queue.push(thread_cmd);
            m_async_command_queue_lock.unlock();
        }
        else {
            //Logger::Log("Received main command: " + std::to_string(data.command));
            m_main_command_queue_lock.lock();
            m_main_command_queue.push(thread_cmd);
            m_main_cmd_q_len++;
            m_main_command_queue_lock.unlock();
        }
    //}
    //else {
    //    Logger::Log("User submitted invalid command");
    //}
}

void AsyncServer::PopulateUserList()
{
    m_user_mtx.lock();
    m_Socket_user_list.clear();
    m_Socket_user_list.reserve(m_socket_users.size());
    for (const auto& user : m_socket_users) {
        m_Socket_user_list.emplace_back(user.second);
    }
    m_user_mtx.unlock();
}

uint16_t AsyncServer::Get_New_UDP_ID()
{
    uint16_t newNum = HashHelper::RandomNumber(0, UINT16_MAX);
    if (Has_UDP_ID(newNum)) {
        newNum = Get_New_UDP_ID();
    }
    return newNum;
}

void AsyncServer::DoProcess(std::string socket_user, Data data) {

    if (!HasCommand(data.command)) {
        Logger::LogWarning("Invalid command submitted");
        return;
    }

    NetCommand command_obj = m_commands[data.command];

    m_user_mtx.lock();
    SocketUser* user_obj;
    bool hasPlayer = HasPlayerSession(socket_user);
    if (hasPlayer) {
        user_obj = m_socket_users[socket_user].get();
    }
    m_user_mtx.unlock();

    if (hasPlayer)
        command_obj.Callback(command_obj.Obj_Ptr, *user_obj, data);
}

void AsyncServer::System_Cmd(SocketUser& socket_user, Data data)
{
    uint8_t sub_command = data.Buffer[0];
    //Logger::Log("Sub Command: " + std::to_string(sub_command));

    data.Buffer = BufferUtils::RemoveFront(Remove_CMD, data.Buffer);

    switch (sub_command) {
    case 0x02: // disconnect
        RemovePlayer(socket_user.SessionToken);
        break;
    case 0x03: // ping
        socket_user.ResetPingCounter();
        socket_user.Send(OpCodes::Client::System_Reserved, std::vector<uint8_t>({ 0x03, 0x01 }), Protocal_Tcp);
        //Logger::Log("Received ping for client: " + socket_user->SessionToken);
        break;
    case 0x04: // set UDP client port
        uint16_t port = *((uint16_t*)data.Buffer.data());
        socket_user.Set_Client_UDP_Port(port);
        break;
    }
}


