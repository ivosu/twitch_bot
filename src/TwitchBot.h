//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_TWITCHBOT_H
#define TWITCH_IRC_TWITCHBOT_H

#include "Message.h"
#include <string>
#include <pplx/pplxtasks.h>
#include <cpprest/ws_client.h>

class TwitchBot {
  public:
	bool connect();

	bool login(const std::string& nickname, const std::string& auth, ...);

	pplx::task<void> sendMessage(const std::string& message, const std::string& channel);

	pplx::task<void> sendMessage(const std::string& message);

	std::pair<bool, std::unique_ptr<Message>> readMessage(unsigned int timeout = 0);

	bool partChannel();

	bool joinChannel(const std::string& channel);

	void onCommand(const std::string& sender, const std::string& command, const std::string& restOfCommand);

	bool capReq(const std::string& stuff);

  private:
	std::pair<bool, std::string> readSimpleMessage();

	pplx::task<void> sendSimpleMessage(const std::string& message);

	web::websockets::client::websocket_client m_Client;
	std::string m_Nickname;
	bool m_LoggedIn = false;
	bool m_JoinedChannel = false;
	std::string m_Channel;
};

#endif //TWITCH_IRC_TWITCHBOT_H
