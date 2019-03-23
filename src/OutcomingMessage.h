//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_OUTCOMINGMESSAGE_H
#define TWITCH_IRC_OUTCOMINGMESSAGE_H

#include "Message.h"

class OutcomingMessage : public Message {
  public:
	OutcomingMessage(const std::map<std::string, std::variant<std::string, void>>& tags, const std::string& prefix,
					 const std::string& command, const std::vector<std::string>& params);
};


#endif //TWITCH_IRC_OUTCOMINGMESSAGE_H
