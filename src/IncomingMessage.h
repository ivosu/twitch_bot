//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_INCOMINGMESSAGE_H
#define TWITCH_IRC_INCOMINGMESSAGE_H

#include "Message.h"

class IncomingMessage : public Message {
  public:
	IncomingMessage(const std::string& rawMessage);
};


#endif //TWITCH_IRC_INCOMINGMESSAGE_H
