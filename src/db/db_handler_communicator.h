#ifndef TWITCH_IRC_DB_HANDLER_COMMUNICATOR_H
#define TWITCH_IRC_DB_HANDLER_COMMUNICATOR_H

#include <string>
#include "../handling/handlers/event_handler.h"

class db_handler_communicator {
  public:
	virtual bool save_or_update_handler(const event_handler& handler, const std::string& channel) = 0;

	virtual std::vector<std::shared_ptr<event_handler>> load_handlers(const std::string& channel) = 0;

	virtual std::shared_ptr<event_handler>
	load_handler(const std::string& channel, event_handler::event_type event_type) = 0;

  protected:
	db_handler_communicator() = default;
};


#endif //TWITCH_IRC_DB_HANDLER_COMMUNICATOR_H
