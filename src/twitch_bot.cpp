//
// Created by strejivo on 3/22/19.
//

#include "twitch_bot.h"

using std::string;
using std::vector;
using irc::message;

twitch_bot::twitch_bot() : m_irc_client(U("wss://irc-ws.chat.twitch.tv:443")) {}

bool twitch_bot::login(const std::string& nickname, const std::string& auth, ...) {
	assert(!m_logged_in);
	message passMessage({}, "", "PASS", {auth});
	message nickMessage({}, "", "NICK", {nickname});
	if (m_irc_client.send_message(passMessage).wait() != pplx::task_status::completed ||
			m_irc_client.send_message(nickMessage).wait() != pplx::task_status::completed)
		return false;
	message retMessage = m_irc_client.read_message();
	if (retMessage.command() != "001" || retMessage.prefix() != "tmi.twitch.tv" ||
			retMessage.params() != vector<string>{nickname, "Welcome, GLHF!"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "002" || retMessage.prefix() != "tmi.twitch.tv" ||
			retMessage.params() != vector<string>{nickname, "Your host is tmi.twitch.tv"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "003" || retMessage.prefix() != "tmi.twitch.tv" ||
			retMessage.params() != vector<string>{nickname, "This server is rather new"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "004" || retMessage.prefix() != "tmi.twitch.tv" ||
			retMessage.params() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "375" || retMessage.prefix() != "tmi.twitch.tv" ||
			retMessage.params() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "372" || retMessage.prefix() != "tmi.twitch.tv" ||
			retMessage.params() != vector<string>{nickname, "You are in a maze of twisty passages, all alike."})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "376" || retMessage.prefix() != "tmi.twitch.tv" ||
			retMessage.params() != vector<string>{nickname, ">"})
		return false;
	m_logged_in = true;
	m_nickname = nickname;
	return true;
}

pplx::task<void> twitch_bot::send_message(const std::string& message, const std::string& channel) {
	assert(m_logged_in);
	return m_irc_client.send_message(message::private_message(message, channel));
}

pplx::task<void> twitch_bot::send_message(const std::string& message) {
	assert(m_logged_in);
	assert(m_joined_channel);
	return send_message(message, m_channel);
}

message twitch_bot::read_message(unsigned int timeout) {
	assert(m_logged_in);
	assert(m_joined_channel);
	auto tmp_message = m_irc_client.read_message(timeout);
	if (tmp_message.command() == "PING" && tmp_message.params() == vector<string>{"tmi.twitch.tv"}) {
		std::cout<<"PING"<<std::endl;
		m_irc_client.send_message(message({}, "", "PONG", {"tmi.twitch.tv"}));
		return read_message(timeout); // TODO should subtract time elapsed so far
	}
	return tmp_message;
}

bool twitch_bot::part_channel() {
	assert(m_logged_in);
	assert(m_joined_channel);
	message partMessage({}, "", "PART", {"#" + m_channel});
	if (m_irc_client.send_message(partMessage).wait() != pplx::task_status::completed)
		return false;
	m_joined_channel = false;
	return true;
}

bool twitch_bot::join_channel(const std::string& channel) {
	assert(m_logged_in);
	if (m_joined_channel)
		part_channel();
	message joinMessage({}, "", "JOIN", {"#" + channel});
	if (m_irc_client.send_message(joinMessage).wait() != pplx::task_status::completed)
		return false;

	message responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "JOIN" || responseMessage.params() != vector<string>{"#" + channel} ||
			responseMessage.prefix() != m_nickname + "!" + m_nickname + "@" + m_nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "353" ||
			responseMessage.params() != vector<string>{m_nickname, "=", "#" + channel, m_nickname} ||
			responseMessage.prefix() != m_nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "366" ||
			responseMessage.params() != vector<string>{m_nickname, "#" + channel, "End of /NAMES list"} ||
			responseMessage.prefix() != m_nickname + ".tmi.twitch.tv")
		return false;
	m_joined_channel = true;
	m_channel = channel;
	return true;
}

void twitch_bot::on_command(const std::string& sender, const std::string& command, const std::string& restOfCommand) {
	if (command == "hello")
		pplx::task<void>([this, sender] {
			send_message("Hello " + sender);
		});
}

bool twitch_bot::cap_req(const std::string& stuff) {// TODO refactor
	m_irc_client.send_message(message({}, "", "CAP", {"REQ", stuff})).wait();
	message message = m_irc_client.read_message();
	return (message.command() == "CAP" && message.prefix() == "tmi.twitch.tv" &&
			message.params() == vector<string>{"*", "ACK", stuff});
}
