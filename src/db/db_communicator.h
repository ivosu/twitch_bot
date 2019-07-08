//
// Created by strejivo on 7/3/19.
//

#ifndef TWITCH_IRC_DB_COMMUNICATOR_H
#define TWITCH_IRC_DB_COMMUNICATOR_H

#include "../irc/message.h"

class db_communicator {
  public:
	virtual bool saveMessage(const irc::message& message) = 0;

	virtual std::vector<irc::message> loadMessages(const std::string& querry) = 0;

  protected:
	db_communicator( const std::string& host, uint16_t port, const std::string& username, const std::string auth ){};
};


#endif //TWITCH_IRC_DB_COMMUNICATOR_H
