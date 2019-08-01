//
// Created by strejivo on 7/16/19.
//

#ifndef TWITCH_IRC_PYTHON_EXECUTION_H
#define TWITCH_IRC_PYTHON_EXECUTION_H


#include <pybind11/pytypes.h>
#include "../user_code_execution.h"
#include "../../irc/message.h"

class python_execution : public user_code_execution {
  public:
	void executeOnMessage(const std::string& channel, const std::string& code, const irc::message& message, const twitch_bot& bot,
						  const db_message_communicator& comm) final;
};


#endif //TWITCH_IRC_PYTHON_EXECUTION_H
