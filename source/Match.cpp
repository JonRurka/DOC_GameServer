#include "Match.h"

Match::Match(std::string id)
{
	m_ID = id;
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

}

void Match::ProcessNetCommands()
{
	while (!m_command_queue.empty()) {
		Data data = m_command_queue.front();
		m_command_queue.pop();

		ExecuteNetCommand(data);
	}
}

void Match::ExecuteNetCommand(Data data)
{
	switch (data.command) {
	case 0x00:

		break;
	}
}

void Match::SubmitMatchCommand(Data data)
{
	m_command_queue.push(data);
}
