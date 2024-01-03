#include "Match.h"
#include "Network/AsyncServer.h"

Match::Match(std::string id, uint16_t short_id)
{
	m_ID = id;
	m_short_ID = short_id;
}

Match::~Match()
{
}

void Match::Init()
{
	m_thread = std::thread(Run, this);
}

void Match::Stop()
{
	if (m_running) {
		m_running = false;
		m_thread.join();
	}
}

bool Match::JoinPlayer(Player* player)
{
	// In a isolated match, join any player,
	// In a network match, only join assigned players.


	return true;
}

void Match::GetMatchInfo()
{
}

void Match::Run(Match* mtch)
{
	mtch->ThreadInit();
	mtch->GameLoop();
}

void Match::ThreadInit()
{
	GetMatchInfo();

	m_running = true;
}

void Match::GameLoop()
{
	while (m_running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		AsynUpdate();
	}
}

void Match::AsynUpdate()
{
	ProcessNetCommands();
}

void Match::ProcessNetCommands()
{
	while (!m_command_queue.empty()) {
		NetCommand data = m_command_queue.front();
		m_command_queue.pop();

		ExecuteNetCommand(data.user, data.data);
	}
}

void Match::ExecuteNetCommand(AsyncServer::SocketUser* user, Data data)
{
	uint8_t sub_command = data.Buffer[0];

	switch (sub_command) {
	case 0x00:

		break;
	}
}

void Match::SubmitMatchCommand(AsyncServer::SocketUser* user, Data data)
{
	NetCommand command;
	command.user = user;
	command.data = data;
	m_command_queue.push(command);
}
