//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_TWITCHBOT_H
#define TWITCH_IRC_TWITCHBOT_H

#include "irc/message.h"
#include "irc/irc_client.h"
#include <string>


class twitch_bot {
  public:

	twitch_bot();

	bool login(const std::string& nickname, const std::string& auth, ...);

	pplx::task<void> send_message(const std::string& message, const std::string& channel);

	pplx::task<void> send_message(const std::string& message);

	irc::message read_message(unsigned int timeout = 0);

	bool part_channel(const std::string& channel);

	bool join_channel(const std::string& channel);

	void on_command(const irc::message& command);

	bool cap_req(const std::string& stuff);

  private:
	irc::irc_client m_irc_client;
	std::string m_nickname;
	bool m_logged_in = false;
	bool m_joined_channel = false;
	std::string m_channel;
};

#endif //TWITCH_IRC_TWITCHBOT_H
