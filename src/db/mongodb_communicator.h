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

	bool save_message(const irc::message& message) override;

	std::vector<irc::message> load_messages(const std::string& query) override;

	std::vector<irc::message> load_messages(const bsoncxx::document::view_or_value& query);
  private:
	mongocxx::client m_client;

	mongocxx::collection m_messages_collection;
};


#endif //TWITCH_IRC_MONGODB_COMMUNICATOR_H
