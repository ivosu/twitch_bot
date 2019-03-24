//
// Created by strejivo on 3/24/19.
//

#ifndef TWITCH_IRC_IRCCLIENT_H
#define TWITCH_IRC_IRCCLIENT_H

#include <string>
#include <pplx/pplxtasks.h>
#include <cpprest/ws_client.h>
#include "Message.h"

class IRCClient {
  public:
	IRCClient(std::string host);

	pplx::task<void> sendMessage(const Message& message);

	Message readMessage(size_t timeout = 0);
  private:
	std::queue<Message> m_QueuedMessaged;
	web::websockets::client::websocket_client m_Client;
};


#endif //TWITCH_IRC_IRCCLIENT_H
