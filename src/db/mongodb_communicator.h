//
// Created by strejivo on 7/3/19.
//

#ifndef TWITCH_IRC_MONGODB_COMMUNICATOR_H
#define TWITCH_IRC_MONGODB_COMMUNICATOR_H

#include "db_communicator.h"
#include "mongocxx/client.hpp"
#include "bsoncxx/types.hpp"
#include <mongocxx/client.hpp>

class mongodb_communicator : public db_communicator {
  public:
	mongodb_communicator(const std::string& host, uint16_t port, const std::string& username, const std::string& auth);

	bool saveMessage(const irc::message& message) override;

	std::vector<irc::message> loadMessages(const std::string& querry) override;
  private:
	mongocxx::client m_client;
};


#endif //TWITCH_IRC_MONGODB_COMMUNICATOR_H
