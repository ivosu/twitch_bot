//
// Created by strejivo on 3/22/19.
//

#include "TwitchBot.h"

TwitchBot::TwitchBot() : m_ircClient(U("wss://irc-ws.chat.twitch.tv:443")) {}

bool TwitchBot::login(const std::string& nickname, const std::string& auth, ...) {
	assert(!m_LoggedIn);
	Message passMessage({}, "", "PASS", {auth});
	Message nickMessage({}, "", "NICK", {nickname});
	if (m_ircClient.sendMessage(passMessage).wait() != pplx::task_status::completed ||
		m_ircClient.sendMessage(nickMessage).wait() != pplx::task_status::completed)
		return false;
	Message retMessage = m_ircClient.readMessage();
	if (retMessage.getCommand() != "001" || retMessage.getPrefix() != "tmi.twitch.tv" ||
		retMessage.getParams() != vector<string>{nickname, "Welcome, GLHF!"})
		return false;
	retMessage = m_ircClient.readMessage();
	if (retMessage.getCommand() != "002" || retMessage.getPrefix() != "tmi.twitch.tv" ||
		retMessage.getParams() != vector<string>{nickname, "Your host is tmi.twitch.tv"})
		return false;
	retMessage = m_ircClient.readMessage();
	if (retMessage.getCommand() != "003" || retMessage.getPrefix() != "tmi.twitch.tv" ||
		retMessage.getParams() != vector<string>{nickname, "This server is rather new"})
		return false;
	retMessage = m_ircClient.readMessage();
	if (retMessage.getCommand() != "004" || retMessage.getPrefix() != "tmi.twitch.tv" ||
		retMessage.getParams() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_ircClient.readMessage();
	if (retMessage.getCommand() != "375" || retMessage.getPrefix() != "tmi.twitch.tv" ||
		retMessage.getParams() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_ircClient.readMessage();
	if (retMessage.getCommand() != "372" || retMessage.getPrefix() != "tmi.twitch.tv" ||
		retMessage.getParams() != vector<string>{nickname, "You are in a maze of twisty passages, all alike."})
		return false;
	retMessage = m_ircClient.readMessage();
	if (retMessage.getCommand() != "376" || retMessage.getPrefix() != "tmi.twitch.tv" ||
		retMessage.getParams() != vector<string>{nickname, ">"})
		return false;
	m_LoggedIn = true;
	m_Nickname = nickname;
	return true;
}

pplx::task<void> TwitchBot::sendMessage(const std::string& message, const std::string& channel) {
	assert(m_LoggedIn);
	Message privmsgMessage({}, "", "PRIVMSG", {"#" + channel, ":" + message});
	return m_ircClient.sendMessage(privmsgMessage);
}

pplx::task<void> TwitchBot::sendMessage(const std::string& message) {
	assert(m_LoggedIn);
	assert(m_JoinedChannel);
	return sendMessage(message, m_Channel);
}

Message TwitchBot::readMessage(unsigned int timeout) {
	assert(m_LoggedIn);
	assert(m_JoinedChannel);
	auto message = m_ircClient.readMessage(timeout);
	if (message.getCommand() == "PING" && message.getParams() == vector<string>{"tmi.twitch.tv"}) {
		std::cout<<"PING"<<std::endl;
		m_ircClient.sendMessage(Message({}, "", "PONG", {"tmi.twitch.tv"}));
		return readMessage(timeout); // TODO should subtract time elapsed so far
	}
	return message;
	//auto tmp = readSimpleMessage();
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
	Message partMessage({}, "", "PART", {"#" + m_Channel});
	if (m_ircClient.sendMessage(partMessage).wait() != pplx::task_status::completed)
		return false;
	m_JoinedChannel = false;
	return true;
}

bool TwitchBot::joinChannel(const std::string& channel) {
	assert(m_LoggedIn);
	if (m_JoinedChannel)
		partChannel();
	Message joinMessage({}, "", "JOIN", {"#" + channel});
	if (m_ircClient.sendMessage(joinMessage).wait() != pplx::task_status::completed)
		return false;

	Message responseMessage = m_ircClient.readMessage();
	if (responseMessage.getCommand() != "JOIN" || responseMessage.getParams() != vector<string>{"#" + channel} ||
		responseMessage.getPrefix() != m_Nickname + "!" + m_Nickname + "@" + m_Nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_ircClient.readMessage();
	if (responseMessage.getCommand() != "353" || responseMessage.getParams() != vector<string>{m_Nickname, "=", "#" + channel, m_Nickname} ||
		responseMessage.getPrefix() != m_Nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_ircClient.readMessage();
	if (responseMessage.getCommand() != "366" ||
		responseMessage.getParams() != vector<string>{m_Nickname, "#" + channel, "End of /NAMES list"} ||
		responseMessage.getPrefix() != m_Nickname + ".tmi.twitch.tv")
		return false;

	/*
	if (tmp.second ==
		":" + m_Nickname + ".tmi.twitch.tv 353 " + m_Nickname + " = #" + channel + " :" + m_Nickname + "\r\n"
																									   ":" +
		m_Nickname + ".tmi.twitch.tv 366 " + m_Nickname + " #" + channel + " :End of /NAMES list\r\n") {

	}*/
	m_JoinedChannel = true;
	m_Channel = channel;
	return true;
}

void TwitchBot::onCommand(const std::string& sender, const std::string& command, const std::string& restOfCommand) {
	if (command == "hello")
		pplx::task<void>([this, sender] {
			sendMessage("Hello " + sender);
		});
}

bool TwitchBot::capReq(const std::string& stuff) {// TODO refactor
	m_ircClient.sendMessage(Message({}, "", "CAP", {"REQ", stuff})).wait();
	Message message = m_ircClient.readMessage();
	return (message.getCommand() == "CAP" && message.getPrefix() == "tmi.twitch.tv" &&
			message.getParams() == vector<string>{"*", "ACK", stuff});
}
