#include "message.h"
#include <algorithm>
#include <assert.h>

using std::string;
using std::vector;
using std::map;
using std::optional;
using std::pair;
using std::make_pair;
using std::nullopt;
using std::replace;
using std::shared_ptr;
using std::make_shared;
using irc::message;

namespace irc_parsing {
	static string parseKey(string::const_iterator& it, const string::const_iterator& end) {
		string key;
		while (it != end &&
			   (isalnum(*it) || *it == '-' || *it == '/' || *it == '+')) // TODO make precisely as in rfc1459
			key.push_back(*it++);
		assert(!key.empty());
		return key;
	}

	static string parseTagValue(string::const_iterator& it, const string::const_iterator& end) {
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

	static pair<string, optional<string>> parseTag(string::const_iterator& it, const string::const_iterator& end) {
		string key = parseKey(it, end);
		if (it != end && *it == '=') {
			it++;
			return make_pair(key, parseTagValue(it, end));
		} else
			return make_pair(key, nullopt);
	}

	static map<string, optional<string>> parseTags(string::const_iterator& it, const string::const_iterator& end) {
		map<string, optional<string>> parsedTags;
		if (it == end || *it != '@') {
			return parsedTags;
		}
		it++;
		parsedTags.insert(parseTag(it, end));
		while (it != end && *it == ';') {
			it++;
			parsedTags.insert(parseTag(it, end));
		}
		assert(it != end && *it == ' ');
		it++;
		while (it != end && *it == ' ')
			it++;
		return parsedTags;
	}

	static string parseCommand(string::const_iterator& it, const string::const_iterator& end) {
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

	static string parseMiddleParam(string::const_iterator& it, const string::const_iterator& end) {
		string param;
		while (it != end && *it != ' ' && *it != '\r' && *it != '\n' && *it != '\0')
			param.push_back(*it++);
		return param;
	}

	static string parseTrailingParam(string::const_iterator& it, const string::const_iterator& end) {
		string param;
		while (it != end && *it != '\r' && *it != '\n' && *it != '\0')
			param.push_back(*it++);
		return param;
	}

	static vector<string> parseParams(string::const_iterator& it, const string::const_iterator& end) {
		vector<string> parsedParams;
		while (it != end && *it == ' ') {
			it++;
			if (*it != ':')
				parsedParams.push_back(parseMiddleParam(it, end));
			else {
				it++;
				parsedParams.push_back(parseTrailingParam(it, end));
				break;
			}
		}
		return parsedParams;
	}

	static string parsePrefix(string::const_iterator& it, const string::const_iterator& end) {
		string prefix;
		if (it == end || *it != ':')
			return prefix;
		it++;
		while (it != end && *it != ' ')
			prefix.push_back(*it++);
		assert(it != end);
		assert(*it == ' ');
		it++;
		return prefix;
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
	m_tags = irc_parsing::parseTags(it, end);
	m_prefix = irc_parsing::parsePrefix(it, end);
	m_command = irc_parsing::parseCommand(it, end);
	m_params = irc_parsing::parseParams(it, end);
	assert(it != end && *it == '\r');
	*it++;
	assert(it != end && *it == '\n');
	*it++;
	assert(it == end);
}

message message::private_message(const string& message_text, const string& channel) {
	return message({}, "", "PRIVMSG", {"#" + channel, message_text});
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
	if (!m_prefix.empty()) {
		rawMessage += ":" + m_prefix + ' ';
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

message message::pass_message(const string& password) {
	return message({}, "", "PASS", {password});
}

message message::nick_message(const string& nickname) {
	return message({}, "", "NICK", {nickname});
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
	return message({}, "", "JOIN", {channels_param, keys_param});
}

message message::part_message(const vector<string>& channels) {
	string channels_param;
	assert(!channels.empty());
	auto c_it = channels.cbegin();
	channels_param = "#" + *c_it;
	for (c_it++; c_it != channels.cend(); c_it++)
		channels_param+=",#"+ *c_it;
	return message({}, "", "PART", {channels_param});
}

message message::pong_message(const string& daemon) {
	return message({}, "", "PONG", {daemon});
}

message message::pong_message(const string& daemon1, const string& daemon2) {
	return message({}, "", "PONG", {daemon1, daemon2});
}
