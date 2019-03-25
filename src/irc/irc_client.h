//
// Created by strejivo on 3/24/19.
//

#ifndef TWITCH_IRC_IRCCLIENT_H
#define TWITCH_IRC_IRCCLIENT_H

#include <string>
#include <pplx/pplxtasks.h>
#include <cpprest/ws_client.h>
#include "message.h"

namespace irc {
	class irc_client {
	  public:
		irc_client(std::string host);

		pplx::task<void> send_message(const message& message_to_send);

		message read_message(size_t timeout = 0);

	  private:
		std::queue<message> m_queued_messaged;
		web::websockets::client::websocket_client m_client;
	};
}


#endif //TWITCH_IRC_IRCCLIENT_H
