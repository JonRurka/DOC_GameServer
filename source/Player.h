#pragma once

#include "stdafx.h"
#include "IUser.h"

class Player : public IUser {
public:

	Player();
	~Player();

	bool SetIdentity(std::string json_identity);

	std::string Get_UserName() {
		return m_userName;
	}

	std::string Get_Distribution_UserID() {
		return m_distro_userID;
	}

	uint32_t Get_UserID() {
		return m_UserID;
	}

	int Get_Distributor() {
		return m_distributor;
	}

private:

	std::string m_userName;
	std::string m_distro_userID;
	uint32_t m_UserID;
	int m_distributor;

};