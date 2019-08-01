//
// Created by strejivo on 7/18/19.
//

#ifndef TWITCH_IRC_USER_CODE_EXECUTION_H
#define TWITCH_IRC_USER_CODE_EXECUTION_H


#include "../irc/message.h"
#include "../twitch_bot.h"
#include "../db/db_message_communicator.h"

class user_code_execution {
  public:
	virtual void executeOnMessage(const std::string& channel, const std::string& code, const irc::message& message, const twitch_bot& bot,
								  const db_message_communicator& comm) = 0;

  protected:
	user_code_execution() = default;
};


#endif //TWITCH_IRC_USER_CODE_EXECUTION_H
