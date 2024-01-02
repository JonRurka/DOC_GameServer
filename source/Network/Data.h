#pragma once

#include "../stdafx.h"

typedef unsigned int Protocal;

#define Protocal_Tcp (Protocal)0
#define Protocal_Udp (Protocal)1

class Data {
public:
    Protocal Type;
    uint8_t command;
    std::vector<uint8_t> Buffer;
    std::string Input;

    Data(Protocal type, uint8_t cmd, std::vector<uint8_t> data) {
        Type = type;
        command = cmd;
        Buffer = data;
        Input = std::string(Buffer.begin(), Buffer.end());
    }

    Data(){}
};