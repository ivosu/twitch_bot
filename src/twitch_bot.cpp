//
// Created by strejivo on 3/22/19.
//

#include "twitch_bot.h"

using std::string;
using std::vector;
using std::back_inserter;
using std::nullopt;
using irc::message;
using std::map;
using std::optional;


string twitch_bot::get_user_name_from_user_notice_tags(const map<string, optional<string>>& tags) {
	auto display_name_it = tags.find("display-name");
	if (display_name_it != tags.cend() && display_name_it->second.has_value() &&
		!display_name_it->second.value().empty())
		return display_name_it->second.value();
	else {
		auto login_it = tags.find("login");
		if (login_it != tags.cend() && login_it->second.has_value() && !login_it->second.value().empty())
			return login_it->second.value();
		else
			assert(false);
	}
}

string twitch_bot::get_gifted_recipient_user_name(const map<string, optional<string>>& tags) {
	auto display_name_it = tags.find("msg-param-recipient-display-name");
	if (display_name_it != tags.end() && display_name_it->second.has_value() &&
		!display_name_it->second.value().empty())
		return display_name_it->second.value();
	else {
		auto login_it = tags.find("msg-param-recipient-user-name");
		if (login_it != tags.end() && login_it->second.has_value() && !login_it->second.value().empty())
			return login_it->second.value();
		else
			assert(false);
	}
}

string twitch_bot::get_user_name_private_message(const message& message) {
	assert(message.command() == "PRIVMSG");
	auto tags = message.tags();
	auto display_name_it = tags.find("display-name");
	if (display_name_it != tags.end() && display_name_it->second.has_value() &&
		!display_name_it->second.value().empty())
		return display_name_it->second.value();
	else {
		assert(message.prefix().has_value());
		return message.prefix().value().main();
	}
}

twitch_bot::twitch_bot() : m_irc_client(U("wss://irc-ws.chat.twitch.tv:443")) {}

bool twitch_bot::login(const std::string& nickname, const std::string& auth, ...) {
	assert(!m_logged_in);
	if (m_irc_client.send_message(message::pass_message(auth)).wait() != pplx::task_status::completed ||
		m_irc_client.send_message(message::nick_message(nickname)).wait() != pplx::task_status::completed)
		return false;
	message retMessage = m_irc_client.read_message();
	if (retMessage.command() != "001" || !retMessage.prefix().has_value() ||
		retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "Welcome, GLHF!"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "002" || !retMessage.prefix().has_value() ||
		retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "Your host is tmi.twitch.tv"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "003" || !retMessage.prefix().has_value() ||
		retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "This server is rather new"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "004" || !retMessage.prefix().has_value() ||
		retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "375" || !retMessage.prefix().has_value() ||
		retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "-"})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "372" || !retMessage.prefix().has_value() ||
		retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		retMessage.params() != vector<string>{nickname, "You are in a maze of twisty passages, all alike."})
		return false;
	retMessage = m_irc_client.read_message();
	if (retMessage.command() != "376" || !retMessage.prefix().has_value() ||
		retMessage.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
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
		!responseMessage.prefix().has_value() || responseMessage.prefix().value().to_irc_prefix() !=
												 m_nickname + "!" + m_nickname + "@" + m_nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "353" ||
		responseMessage.params() != vector<string>{m_nickname, "=", "#" + channel, m_nickname} ||
		!responseMessage.prefix().has_value() ||
		responseMessage.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
		return false;

	responseMessage = m_irc_client.read_message();
	if (responseMessage.command() != "366" ||
		responseMessage.params() != vector<string>{m_nickname, "#" + channel, "End of /NAMES list"} ||
		!responseMessage.prefix().has_value() ||
		responseMessage.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
		return false;
	m_joined_channels.insert(channel);
	return true;
}

void twitch_bot::on_command(const message& command) {

}

void twitch_bot::cap_req(const vector<string>& capabilities) {

	m_irc_client.send_message(message::capability_request_message(capabilities)).wait();
}
