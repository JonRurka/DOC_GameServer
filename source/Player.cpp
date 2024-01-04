#include "Player.h"

#include <boost/json.hpp>

using namespace boost;

Player::Player()
{
}

Player::~Player()
{
}

bool Player::SetIdentity(std::string json_identity)
{
	Logger::Log(json_identity);

	json::parse_options opt;
	opt.allow_trailing_commas = true;

	json::error_code ec;
	json::value json_val = json::parse(json_identity, ec, json::storage_ptr(), opt);

	if (ec) {
		Logger::Log("Failed to parse user identity.");
		return false;
	}

	json::object ident_obj = json_val.as_object();

	m_userName = std::string(ident_obj.at("UserName").as_string());
	m_distro_userID = std::string(ident_obj.at("User_Distro_ID").as_string());
	m_UserID = ident_obj.at("UserID").as_int64();

	//Logger::Log("Is Number: " + std::to_string((int)ident_obj.at("Distributor").is_number()) + ", " + std::to_string((int)ident_obj.at("Distributor").is_int64()));
	m_distributor = ident_obj.at("Distributor").as_int64();

	m_UserID = 0; // from database.

	return true;
}
