#pragma once

#include <stdint.h>

class OpCodes {
public:
	enum class Server : uint8_t {
		System_Reserved = 0xff,
		Submit_Identity = 0x01,
		Match_Command = 0x02,
		Join_Match = 0x03
	};

	enum class Server_Match : uint8_t {
		Debug_Start = 0x01, 
		Update_Orientation = 0x02,
		Player_Event = 0x03
	};

	enum class Client : uint8_t {
		System_Reserved = 0xff,
		Identify_Result = 0x01,
		Join_Match_Result = 0x02,
		Start_Match = 0x03,
		Update_Orientations = 0x04,
		Player_Events = 0x05
	};

	enum class Player_Events : uint8_t {
		None = 0x00,
		Jump = 0x01
	};

};