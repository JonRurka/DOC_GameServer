#include "CommandExecuter.h"
#include "Server_Main.h"
#include "Logger.h"

CommandExecuter::CommandExecuter()
{
    running = false;
    _inputStr_size = 0;
    
    //c_thread = NULL;

}

void CommandExecuter::Run(bool multiThread)
{
    running = true;
    _inputStr_size = 0;
    if (multiThread)
        c_thread = std::thread(&CommandExecuter::Loop, this);
}

void CommandExecuter::Loop()
{
    char c, i = 0;

    do
    {
        Process();
        Sleep(1);
    } while (running);

}

void CommandExecuter::Process()
{
    if (!running)
        return;

    char c;

    if (!kbhit())
        return;

    c = getche();

    if (c == '\n' || c == '\r')
    {
        printf("\n");
        //std::string output = Logger::InputStr();
        //Logger::InputStr("", 0);
        Logger::ResetInput();


        Server_Main::Instance()->SetCommand(GetCurrentInput());
        memset(_inputStr, 0, INPUT_MAX);
        _inputStr_size = 0;
        Server_Main::Instance()->SetCurrentCommand(GetCurrentInput());
    }
    else
    {
        //printf("%c", c);
        _inputStr[_inputStr_size] = c;
        _inputStr_size++;
        Server_Main::Instance()->SetCurrentCommand(GetCurrentInput());
        //Logger::InputStr(_inputStr, i + 1);
    }
}

void CommandExecuter::Stop()
{
    running = false;

}
