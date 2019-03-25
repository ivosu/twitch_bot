//
// Created by strejivo on 3/22/19.
//

#include "twitch_bot.h"

using std::string;
using std::vector;
using std::nullopt;
using irc::message;

twitch_bot::twitch_bot() : m_irc_client(U("wss://irc-ws.chat.twitch.tv:443")) {}

bool twitch_bot::login(const std::string& nickname, const std::string& auth, ...) {
	assert(!m_logged_in);
	if (m_irc_client.send_message(message::pass_message(auth)).wait() != pplx::task_status::completed ||
		m_irc_client.send_message(message::nick_message(nickname)).wait() != pplx::task_status::completed)
		return false;
	message retMessage = m_irc_client.read_message();
	if (retMessage.command() != "001" || !retMessage.prefix().has_value() || retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "Welcome, GLHF!"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "002" || !retMessage.prefix().has_value() || retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "Your host is tmi.twitch.tv"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "003" || !retMessage.prefix().has_value() || retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "This server is rather new"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "004" || !retMessage.prefix().has_value() || retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "375" || !retMessage.prefix().has_value() || retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "372" || !retMessage.prefix().has_value() || retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "You are in a maze of twisty passages, all alike."})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "376" || !retMessage.prefix().has_value() || retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, ">"})
		return false;
	m_logged_in = true;
	m_nickname = nickname;
	return true;
}

pplx::task<void> twitch_bot::send_message(const std::string& message, const std::string& channel) {
	assert(m_logged_in);
	assert(m_joined_channels.find(channel) != m_joined_channels.end());
	return m_irc_client.send_message(message::private_message(message, channel));
}

message twitch_bot::read_message(unsigned int timeout) {
	assert(m_logged_in);
	auto tmp_message = m_irc_client.read_message(timeout);
	if (tmp_message.command() == "PING" && tmp_message.params() == vector<string>{"tmi.twitch.tv"}) {
		std::cout << "PING" << std::endl;
		m_irc_client.send_message(message::pong_message("tmi.twitch.tv"));
		return read_message(timeout); // TODO should subtract time elapsed so far
	}
	return tmp_message;
}

bool twitch_bot::part_channel(const string& channel) {
	assert(m_logged_in);
	assert(m_joined_channels.find(channel) != m_joined_channels.end());
	if (m_irc_client.send_message(message::part_message({channel})).wait() == pplx::task_status::completed) {
		m_joined_channels.erase(channel);
		return true;
	}
	return false;
}

bool twitch_bot::join_channel(const std::string& channel) {
	assert(m_logged_in);
	if (m_joined_channels.find(channel) != m_joined_channels.end())
		return true;
	if (m_irc_client.send_message(message::join_message({channel})).wait() != pplx::task_status::completed)
		return false;

	message responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "JOIN" || responseMessage.params() != vector<string>{"#" + channel} ||
		!responseMessage.prefix().has_value() || responseMessage.prefix().value().to_irc_prefix() != m_nickname + "!" + m_nickname + "@" + m_nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "353" ||
		responseMessage.params() != vector<string>{m_nickname, "=", "#" + channel, m_nickname} ||
	  !responseMessage.prefix().has_value() || responseMessage.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "366" ||
		responseMessage.params() != vector<string>{m_nickname, "#" + channel, "End of /NAMES list"} ||
	  !responseMessage.prefix().has_value() || responseMessage.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
		return false;
	m_joined_channels.insert(channel);
	return true;
}

void twitch_bot::on_command(const message& command) {

}

void twitch_bot::cap_req(const std::string& stuff) {// TODO refactor
	m_irc_client.send_message(message({}, nullopt, "CAP", {"REQ", stuff})).wait();
}
