//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_TWITCHBOT_H
#define TWITCH_IRC_TWITCHBOT_H

#include "irc/Message.h"
#include "irc/IRCClient.h"
#include <string>


class TwitchBot {
  public:

	TwitchBot();

	bool login(const std::string& nickname, const std::string& auth, ...);

	pplx::task<void> sendMessage(const std::string& message, const std::string& channel);

	pplx::task<void> sendMessage(const std::string& message);

	Message readMessage(unsigned int timeout = 0);

	bool partChannel();

	bool joinChannel(const std::string& channel);

	void onCommand(const std::string& sender, const std::string& command, const std::string& restOfCommand);

	bool capReq(const std::string& stuff);

  private:
	IRCClient m_ircClient;
	std::string m_Nickname;
	bool m_Connected = false;
	bool m_LoggedIn = false;
	bool m_JoinedChannel = false;
	std::string m_Channel;
};

#endif //TWITCH_IRC_TWITCHBOT_H
