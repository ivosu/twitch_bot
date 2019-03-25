#include "message.h"
#include <algorithm>
#include <assert.h>

using std::string;
using std::vector;
using std::map;
using std::optional;
using std::nullopt;
using std::make_optional;
using std::pair;
using std::make_pair;
using std::replace;
using std::shared_ptr;
using std::make_shared;
using irc::message;
using irc::prefix::prefix;

#define SKIP_WHITESPACES(it, end) do { it++; } while(it != end && *it == ' ')

namespace irc_parsing {
	static string parse_key(string::const_iterator& it, const string::const_iterator& end) {
		string key;
		while (it != end &&
			   (isalnum(*it) || *it == '-' || *it == '/' || *it == '+')) // TODO make precisely as in rfc1459
			key.push_back(*it++);
		assert(!key.empty());
		return key;
	}

	static string parse_tag_value(string::const_iterator& it, const string::const_iterator& end) {
		string tagValue;
		while (it != end && *it != ';' && *it != ' ' && *it != '\r' && *it != '\n' && *it != '\0') {
			if (*it == '\\') {
				it++;
				if (it != end) {
					switch (*it) {
						case ':':
							tagValue.push_back(';');
							break;
						case 's':
							tagValue.push_back(' ');
							break;
						case '\\':
							tagValue.push_back('\\');
							break;
						case 'r':
							tagValue.push_back('\r');
							break;
						case 'n':
							tagValue.push_back('\n');
							break;
						default:
							throw "Unknown escape sequence"; // TODO
					}
					it++;
				}
			} else tagValue.push_back(*it++);
		}
		return tagValue;
	}

	static pair<string, optional<string>> parse_tag(string::const_iterator& it, const string::const_iterator& end) {
		string key = parse_key(it, end);
		if (it != end && *it == '=') {
			it++;
			return make_pair(key, parse_tag_value(it, end));
		} else
			return make_pair(key, nullopt);
	}

	static map<string, optional<string>> parse_tags(string::const_iterator& it, const string::const_iterator& end) {
		map<string, optional<string>> parsedTags;
		if (it == end || *it != '@') {
			return parsedTags;
		}
		it++;
		parsedTags.insert(parse_tag(it, end));
		while (it != end && *it == ';') {
			it++;
			parsedTags.insert(parse_tag(it, end));
		}
		assert(it != end && *it == ' ');
		SKIP_WHITESPACES(it, end);
		return parsedTags;
	}

	static string parse_command(string::const_iterator& it, const string::const_iterator& end) {
		string command;
		if (isalpha(*it)) {
			command.push_back(*it++);
			while (it != end && isalpha(*it))
				command.push_back(*it++);
		} else {
			assert(isdigit(*it));
			command.push_back(*it++);
			assert(isdigit(*it));
			command.push_back(*it++);
			assert(isdigit(*it));
			command.push_back(*it++);
		}
		return command;
	}

	static string parse_middle_param(string::const_iterator& it, const string::const_iterator& end) {
		string param;
		while (it != end && *it != ' ' && *it != '\r' && *it != '\n' && *it != '\0')
			param.push_back(*it++);
		return param;
	}

	static string parse_trailing_param(string::const_iterator& it, const string::const_iterator& end) {
		string param;
		while (it != end && *it != '\r' && *it != '\n' && *it != '\0')
			param.push_back(*it++);
		return param;
	}

	static vector<string> parse_params(string::const_iterator& it, const string::const_iterator& end) {
		vector<string> parsedParams;
		while (it != end && *it == ' ') {
			SKIP_WHITESPACES(it, end);
			if(it == end)
				return parsedParams;
			if (*it != ':')
				parsedParams.push_back(parse_middle_param(it, end));
			else {
				it++;
				parsedParams.push_back(parse_trailing_param(it, end));
				break;
			}
		}
		return parsedParams;
	}

	static optional<prefix> parse_prefix(string::const_iterator& it, const string::const_iterator& end) {
		if (it == end || *it != ':')
			return nullopt;
		it++; // Eat ':'
		string main;
		string user;
		optional<string> res_user;
		string host;
		optional<string> res_host;
		while (it != end && *it != ' ' && *it != '!' && *it != '@') // Parse main
			main.push_back(*it++);
		if (it != end && *it == '!') { // User part of prefix is present
			it++; // Eat '!'
			assert(it != end);
			while (it != end && *it != ' ' && *it != '@') // Parse user part
				user.push_back(*it++);
			assert(!user.empty());
			res_user = make_optional(user);
		}
		if (it != end && *it == '@') { // Host part of prefix is present
			it++; // Eat '@'
			assert(it != end);
			while(it != end && *it != ' ') // Parse host part
				host.push_back(*it++);
			assert(!host.empty());
			res_host = make_optional(host);
		}
		assert(it != end && *it == ' ');
		SKIP_WHITESPACES(it, end);

		return prefix(main, res_user, res_host);
	}
}

static string escapeTagValue(const string& tagValue) {
	string escapedTagValue;
	for (char c : tagValue) {
		switch (c) {
			case ' ':
				escapedTagValue += "\\s";
				break;
			case '\r':
				escapedTagValue += "\\r";
				break;
			case '\n':
				escapedTagValue += "\\n";
				break;
			case '\\':
				escapedTagValue += "\\\\";
				break;
			case ';':
				escapedTagValue += "\\:";
				break;
			default:
				escapedTagValue.push_back(c);
}
	}
	return escapedTagValue;
}

message::message(const string& rawMessage) {
	auto it = rawMessage.begin();
	auto end = rawMessage.end();
	m_tags = irc_parsing::parse_tags(it, end);
	m_prefix = irc_parsing::parse_prefix(it, end);
	m_command = irc_parsing::parse_command(it, end);
	m_params = irc_parsing::parse_params(it, end);
	assert(it != end && *it == '\r');
	*it++;
	assert(it != end && *it == '\n');
	*it++;
	assert(it == end);
}

message message::private_message(const string& message_text, const string& channel) {
	return message({}, nullopt, "PRIVMSG", {"#" + channel, message_text});
}

message message::pass_message(const string& password) {
	return message({}, nullopt, "PASS", {password});
}

message message::nick_message(const string& nickname) {
	return message({}, nullopt, "NICK", {nickname});
}

message message::join_message(const vector<string>& channels, const vector<string>& keys) {
	string channels_param;
	assert(!channels.empty());
	auto c_it = channels.cbegin();
	channels_param = "#" + *c_it;
	for (c_it++; c_it != channels.cend(); c_it++)
		channels_param+=",#"+ *c_it;
	string keys_param;
	if (!keys.empty()) {
		auto k_it = keys.cbegin();
		keys_param = *k_it;
		for (k_it++; k_it != keys.cend(); k_it++)
			keys_param+= "," + *k_it;
	}
	return message({}, nullopt, "JOIN", {channels_param, keys_param});
}

message message::part_message(const vector<string>& channels) {
	string channels_param;
	assert(!channels.empty());
	auto c_it = channels.cbegin();
	channels_param = "#" + *c_it;
	for (c_it++; c_it != channels.cend(); c_it++)
		channels_param+=",#"+ *c_it;
	return message({}, nullopt, "PART", {channels_param});
}

message message::pong_message(const string& daemon) {
	return message({}, nullopt, "PONG", {daemon});
}

message message::pong_message(const string& daemon1, const string& daemon2) {
	return message({}, nullopt, "PONG", {daemon1, daemon2});
}

string message::to_irc_message() const {
	string rawMessage;
	if (!m_tags.empty()) {
		auto it = m_tags.begin();
		rawMessage = '@' + it->first;
		if (it->second.has_value())
			rawMessage += '=' + escapeTagValue(it->second.value());
		it++;
		for (; it != m_tags.end(); it++) {
			rawMessage += ';' + it->first;
			if (it->second.has_value())
				rawMessage += '=' + escapeTagValue(it->second.value());
		}
		rawMessage += ' ';
	}
	if (m_prefix.has_value()) {
		rawMessage += ":" + m_prefix.value().to_irc_prefix() + ' ';
	}
	rawMessage += m_command;
	for (auto it = m_params.begin(); it != m_params.end(); it++) {
		rawMessage += ' ';
		if (it->find(' ') != string::npos) {
			assert(next(it) == m_params.end());
			rawMessage += ":";
		}
		rawMessage += *it;
	}
	rawMessage += "\r\n";
	return rawMessage;
}

string prefix::to_irc_prefix() const {
	string prefix = m_main;
	if (m_user.has_value())
		prefix += "!" + m_user.value();
	if (m_host.has_value())
		prefix += "@" + m_host.value();
	return prefix;
}
