project("GameServer")

set(${PROJECT_NAME}_headers
	stdafx.h
	exe_main.h
	Server_Main.h
	Linux_conio.hpp
	Logger.h
	CommandExecuter.h
	IUser.h
	HashHelper.h
	MatchManager.h
	Match.h
	Player.h
)

set(${PROJECT_NAME}_sources
    exe_main.cpp
	Server_Main.cpp
	Logger.cpp
	CommandExecuter.cpp
	IUser.cpp
	MatchManager.cpp
	Match.cpp
	Player.cpp
)

add_subdirectory(Network)


add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources})

target_link_libraries(GameServer Network)
