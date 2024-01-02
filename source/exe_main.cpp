#include "exe_main.h"
#include "Server_Main.h"

int main()
{

	Server_Main* server = new Server_Main("");
	server->Start();

	return 0;
}
