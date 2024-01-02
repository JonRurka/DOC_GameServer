#pragma once

#include <stdint.h>

class OpCodes {
public:
	enum class Server : uint8_t {
		System_Reserved = 0xff,
		Submit_Identity = 0x01
	};

	enum class Client : uint8_t {
		System_Reserved = 0xff,
		Request_Identity = 0x01
	};

};