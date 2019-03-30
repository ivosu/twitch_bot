//
// Created by strejivo on 3/22/19.
//

#include "twitch_bot.h"

using std::string;
using std::vector;
using std::back_inserter;
using std::nullopt;
using std::optional;
using std::chrono::milliseconds;

using irc::message;
using irc::tags_t;

using pplx::task;
using pplx::task_status::completed;

string twitch_bot::get_user_name_from_user_notice_tags(const tags_t& tags) {
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

string twitch_bot::get_gifted_recipient_user_name(const tags_t& tags) {
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

twitch_bot::twitch_bot() : m_irc_client(U("wss://irc-ws.chat.twitch.tv:443"), true) {}

bool twitch_bot::login(const string& nickname, const string& auth) {
	assert(!m_logged_in);
	if (m_irc_client.send_message(message::pass_message(auth)).wait() != completed ||
		m_irc_client.send_message(message::nick_message(nickname)).wait() != completed)
		return false;
	message response_message = m_irc_client.read_message();
	if (response_message.command() != "001" || !response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		response_message.params() != vector<string>{nickname, "Welcome, GLHF!"})
		return false;
	response_message = m_irc_client.read_message();
	if (response_message.command() != "002" || !response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		response_message.params() != vector<string>{nickname, "Your host is tmi.twitch.tv"})
		return false;
	response_message = m_irc_client.read_message();
	if (response_message.command() != "003" || !response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		response_message.params() != vector<string>{nickname, "This server is rather new"})
		return false;
	response_message = m_irc_client.read_message();
	if (response_message.command() != "004" || !response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		response_message.params() != vector<string>{nickname, "-"})
		return false;
	response_message = m_irc_client.read_message();
	if (response_message.command() != "375" || !response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		response_message.params() != vector<string>{nickname, "-"})
		return false;
	response_message = m_irc_client.read_message();
	if (response_message.command() != "372" || !response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		response_message.params() != vector<string>{nickname, "You are in a maze of twisty passages, all alike."})
		return false;
	response_message = m_irc_client.read_message();
	if (response_message.command() != "376" || !response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
		response_message.params() != vector<string>{nickname, ">"})
		return false;
	response_message = m_irc_client.read_message();
	if (response_message.command() != "GLOBALUSERSTATE")
		return false;
	m_logged_in = true;
	m_nickname = nickname;
	return true;
}

task<void> twitch_bot::send_message(const string& message, const string& channel) {
	assert(m_logged_in);
	assert(m_joined_channels.find(channel) != m_joined_channels.end());
	return m_irc_client.send_message(message::private_message(message, channel));
}

message twitch_bot::read_message(const milliseconds& timeout) {
	assert(m_logged_in);
	return m_irc_client.read_message(timeout);
}

message twitch_bot::read_message() {
	assert(m_logged_in);
	return m_irc_client.read_message();
}

bool twitch_bot::part_channel(const string& channel) {
	assert(m_logged_in);
	assert(m_joined_channels.find(channel) != m_joined_channels.end());
	if (m_irc_client.send_message(message::part_message({channel})).wait() == completed) {
		m_joined_channels.erase(channel);
		return true;
	}
	return false;
}

bool twitch_bot::join_channel(const string& channel) {
	assert(m_logged_in);
	if (m_joined_channels.find(channel) != m_joined_channels.end())
		return true;
	if (m_irc_client.send_message(message::join_message({channel})).wait() != completed)
		return false;

	message response_message = m_irc_client.read_message();
	if (response_message.command() != "JOIN" || response_message.params() != vector<string>{"#" + channel} ||
		!response_message.prefix().has_value() || response_message.prefix().value().to_irc_prefix() !=
												 m_nickname + "!" + m_nickname + "@" + m_nickname + ".tmi.twitch.tv")
		return false;

	response_message = m_irc_client.read_message();
	if (response_message.command() != "USERSTATE" || response_message.params() != vector<string>{"#" + channel} ||
	  response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" )
		return false;

	response_message = m_irc_client.read_message();
	if (response_message.command() != "ROOMSTATE" || response_message.params() != vector<string>{"#" + channel} ||
	  response_message.prefix().value().to_irc_prefix() != "tmi.twitch.tv" )
		return false;

	response_message = m_irc_client.read_message();
	if (response_message.command() != "353" ||
		response_message.params() != vector<string>{m_nickname, "=", "#" + channel, m_nickname} ||
		!response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
		return false;

	response_message = m_irc_client.read_message();
	if (response_message.command() != "366" ||
		response_message.params() != vector<string>{m_nickname, "#" + channel, "End of /NAMES list"} ||
		!response_message.prefix().has_value() ||
		response_message.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
		return false;
	m_joined_channels.insert(channel);
	return true;
}

void twitch_bot::on_command(const message& command) {

}

bool twitch_bot::cap_req(const vector<string>& capabilities) {
	m_irc_client.send_message(message::capability_request_message(capabilities)).wait();
	message responce_message = m_irc_client.read_message(milliseconds{3000});
	vector<string> expected_ack_params = vector<string>{"*", "ACK"};
	auto c_it = capabilities.cbegin();
	string capabilities_str = *c_it;

	for (c_it++; c_it != capabilities.cend(); c_it++) {
		capabilities_str += ' ' + *c_it;
	}
	expected_ack_params.push_back(capabilities_str);
	if (responce_message.command() != "CAP" || responce_message.params() != expected_ack_params)
		return false;
	m_irc_client.send_message(message::capability_request_end_message()).wait();

}
