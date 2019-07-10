//
// Created by strejivo on 7/3/19.
//

#ifndef TWITCH_IRC_DB_COMMUNICATOR_H
#define TWITCH_IRC_DB_COMMUNICATOR_H

#include "../irc/message.h"

class db_communicator {
  public:
	virtual bool save_message(const irc::message& message) = 0;

	virtual std::vector<irc::message> load_messages(const std::string& query) = 0;

	class invalid_query : public std::exception {
	  public:
		const char* what() const noexcept override {
			return "Invalid query";
		}
	};

  protected:
	db_communicator() = default;
};


#endif //TWITCH_IRC_DB_COMMUNICATOR_H
