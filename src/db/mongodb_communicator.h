#ifndef TWITCH_IRC_MONGODB_COMMUNICATOR_H
#define TWITCH_IRC_MONGODB_COMMUNICATOR_H

#include "db_message_communicator.h"
#include "mongocxx/client.hpp"
#include "bsoncxx/types.hpp"
#include "db_handler_communicator.h"
#include <mongocxx/client.hpp>

class mongodb_communicator : public db_message_communicator, public db_handler_communicator {
  public:
	mongodb_communicator(const std::string& host, uint16_t port, const std::string& username, const std::string& auth);

	bool save_message(const irc::message& message) override;

	std::vector<irc::message> load_messages(const std::string& query) final;

	std::vector<irc::message> load_messages(const bsoncxx::document::view_or_value& query);

	bool save_or_update_handler(const event_handler& handler, const std::string& channel) final;

	std::vector<std::shared_ptr<event_handler>> load_handlers(const std::string& channel) final;

	std::shared_ptr<event_handler> load_handler(const std::string& channel, event_handler::event_type event_type) final;

  private:
	mongocxx::client m_client;

	mongocxx::collection m_messages_collection;

	mongocxx::collection m_handlers_collection;
};


#endif //TWITCH_IRC_MONGODB_COMMUNICATOR_H
