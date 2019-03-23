//
// Created by strejivo on 3/22/19.
//

#include "TwitchBot.h"
#include <regex>

bool TwitchBot::connect() {
	try {
		return m_Client.connect(U("wss://irc-ws.chat.twitch.tv:443")).wait() == pplx::task_status::completed;
	} catch (...) {
		return false;
	}
}

bool TwitchBot::login(const std::string& nickname, const std::string& auth, ...) {
	assert(!m_LoggedIn);
	if (sendSimpleMessage("PASS " + auth).wait() != pplx::task_status::completed ||
		sendSimpleMessage("NICK " + nickname).wait() != pplx::task_status::completed)
		return false;
	auto ret = readSimpleMessage();
	if (!ret.first)
		return false;
	if (ret.second == ":tmi.twitch.tv 001 " + nickname + " :Welcome, GLHF!\r\n"
														 ":tmi.twitch.tv 002 " + nickname +
					  " :Your host is tmi.twitch.tv\r\n"
					  ":tmi.twitch.tv 003 " + nickname + " :This server is rather new\r\n"
														 ":tmi.twitch.tv 004 " + nickname + " :-\r\n"
																							":tmi.twitch.tv 375 " +
					  nickname + " :-\r\n"
								 ":tmi.twitch.tv 372 " + nickname +
					  " :You are in a maze of twisty passages, all alike.\r\n"
					  ":tmi.twitch.tv 376 " + nickname + " :>\r\n") {
		m_LoggedIn = true;
		m_Nickname = nickname;
		return true;
	} else return false;
}

pplx::task<void> TwitchBot::sendMessage(const std::string& message, const std::string& channel) {
	assert(m_LoggedIn);
	return sendSimpleMessage("PRIVMSG #" + channel + " :" + message);
}

pplx::task<void> TwitchBot::sendMessage(const std::string& message) {
	assert(m_LoggedIn);
	assert(m_JoinedChannel);
	return sendMessage(message, m_Channel);
}

std::pair<bool, std::unique_ptr<Message>> TwitchBot::readMessage(unsigned int timeout) {
	assert(m_LoggedIn);
	assert(m_JoinedChannel);
	auto tmp = readSimpleMessage();
	/*if (!tmp.first)
		return std::make_pair(false, nullptr);
	if (tmp.second == "PING :tmi.twitch.tv\r\n") {
		std::cout << "PING!" << std::endl;
		sendSimpleMessage("PONG :tmi.twitch.tv");
		return readMessage(timeout);
	}

	std::smatch match;
	if (!std::regex_match(tmp.second, match, messageRegex) || match[2] != match[3] || match[2] != match[4]) {
		std::cerr<<"Malformated message?"<<std::endl<<tmp.second<<std::endl;
		return std::make_pair(false, nullptr);
	}
	std::string username = match[2], message = match[6], tags = match[1];
	if (match[5] != m_Channel) {
		std::cout<< "WTF "<<tmp.second<<std::endl;
		return readMessage(timeout);
	}
	return std::make_pair(true, std::move(std::make_unique<ChatMessage>(SenderInfo(username, tags), message)));*/

}

bool TwitchBot::partChannel() {
	assert(m_LoggedIn);
	assert(m_JoinedChannel);
	if (sendSimpleMessage("PART  #" + m_Channel).wait() != pplx::task_status::completed)
		return false;
	m_JoinedChannel = false;
	return true;
}

bool TwitchBot::joinChannel(const std::string& channel) {
	assert(m_LoggedIn);
	if (m_JoinedChannel)
		partChannel();
	if (sendSimpleMessage("JOIN #" + channel).wait() != pplx::task_status::completed)
		return false;
	auto tmp = readSimpleMessage();
	if (!tmp.first)
		return false;
	if (tmp.second !=
		":" + m_Nickname + "!" + m_Nickname + "@" + m_Nickname + ".tmi.twitch.tv JOIN #" + channel + "\r\n") {
		return false;
	}
	tmp = readSimpleMessage();
	if (!tmp.first)
		return false;
	if (tmp.second ==
		":" + m_Nickname + ".tmi.twitch.tv 353 " + m_Nickname + " = #" + channel + " :" + m_Nickname + "\r\n"
																									   ":" +
		m_Nickname + ".tmi.twitch.tv 366 " + m_Nickname + " #" + channel + " :End of /NAMES list\r\n") {
		m_JoinedChannel = true;
		m_Channel = channel;
		return true;
	}
	return false;
}

void TwitchBot::onCommand(const std::string& sender, const std::string& command, const std::string& restOfCommand) {
	if (command == "hello")
		pplx::task<void>([this, sender] {
			sendMessage("Hello " + sender);
		});
}

bool TwitchBot::capReq(const std::string& stuff) {// TODO refactor
	sendSimpleMessage("CAP REQ :" + stuff).wait();
	auto tmp = readSimpleMessage();
	if (!tmp.first)
		return false;
	return tmp.second == ":tmi.twitch.tv CAP * ACK :" + stuff + "\r\n";
}

std::pair<bool, std::string> TwitchBot::readSimpleMessage() {
	std::string ret;
	try {
		if (m_Client.receive().then([](web::web_sockets::client::websocket_incoming_message msg) {
			return msg.extract_string();
		}).then([&ret](std::string body) {
			ret = body;
		}).wait() == pplx::task_status::completed) {
			return std::make_pair(true, ret);
		} else return std::make_pair(false, "");
	} catch (...) {
		return std::make_pair(false, "");
	}
}

pplx::task<void> TwitchBot::sendSimpleMessage(const std::string& message) {
	web::web_sockets::client::websocket_outgoing_message msg;
	msg.set_utf8_message(message);
	return m_Client.send(msg);
}
