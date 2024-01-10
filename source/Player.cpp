#include "Player.h"
#include "Server_Main.h"

#include <boost/json.hpp>

using namespace boost;

#define SEND_DEBUG_JUMP 3000

Player::Player()
{
	m_sent_last_jump = Server_Main::GetEpoch();
	m_active_match = nullptr;
	m_match_instance_id = 0;
	m_location = glm::vec3(0, 0, 0);
	m_rotation = glm::quat();
	m_sent_last_jump = Server_Main::GetEpoch();

}

Player::~Player()
{
}

std::shared_ptr<Player> Player::Cast_IUser(std::weak_ptr<IUser> user)
{
	return std::dynamic_pointer_cast<Player>(user.lock());
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

	m_identity.UserName = std::string(ident_obj.at("UserName").as_string());
	m_identity.User_Distro_ID = std::string(ident_obj.at("User_Distro_ID").as_string());
	m_identity.UserID = ident_obj.at("UserID").as_int64();

	//Logger::Log("Is Number: " + std::to_string((int)ident_obj.at("Distributor").is_number()) + ", " + std::to_string((int)ident_obj.at("Distributor").is_int64()));
	m_identity.Distributor = ident_obj.at("Distributor").as_int64();

	return true;
}

bool Player::SetIdentity(PlayerIdentity identity)
{
	m_identity = identity;

	return true;
}

void Player::MatchUpdate(float dt)
{
	/*auto now = Server_Main::GetEpoch();

	if ((now - m_sent_last_jump) > SEND_DEBUG_JUMP) {

		if (m_identity.UserID == 0) {
			Add_Player_Event(OpCodes::Player_Events::Jump);
		}

		m_sent_last_jump = Server_Main::GetEpoch();
	}*/

}
