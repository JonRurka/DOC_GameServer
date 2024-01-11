#include "exe_main.h"
#include "Server_Main.h"

int main()
{
	std::string args;
	Server_Main* server = new Server_Main((char*)args.c_str());
	server->Start();

	return 0;
}
