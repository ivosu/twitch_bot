#include "message.h"
#include <algorithm>
#include <assert.h>

using std::string;
using std::vector;
using std::optional;
using std::nullopt;
using std::make_optional;
using std::pair;
using std::make_pair;
using std::replace;
using std::shared_ptr;
using std::make_shared;

using irc::message;
using irc::tags_t;
using irc::prefix_t;

#define SKIP_WHITESPACES(it, end) do {\
    (it)++;\
} while ((it) != (end) && *(it) == ' ')

#define SKIP_WHITESPACES_THROW_END(it, end, parsed_part) SKIP_WHITESPACES((it), (end));\
    if ((it) == (end))\
        throw message::parsing_error("Message ended while parsing " + string((parsed_part)));

namespace irc_parsing {
	static string parse_key(string::const_iterator& it, const string::const_iterator& end) {
		string key;
		while (isalnum(*it) || *it == '-' || *it == '/' || *it == '+') { // TODO make precisely as in rfc1459
			key.push_back(*it++);
			if (it == end)
				throw message::parsing_error("Message ended while parsing tag key");
		}

		if (key.empty())
			throw message::parsing_error("Empty key in tags parsing");
		return key;
	}

	static string parse_tag_value(string::const_iterator& it, const string::const_iterator& end) {
		string tagValue;
		while (*it != ';' && *it != ' ' && *it != '\r' && *it != '\n' && *it != '\0') {
			if (*it == '\\') {
				it++;
				if (it == end)
					throw message::parsing_error("Message ended while parsing tag value");
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
						throw message::parsing_error("Unknown escape sequence");
				}
				it++;
			} else tagValue.push_back(*it++);
			if (it == end)
				throw message::parsing_error("Message ended while parsing tag value");
		}
		return tagValue;
	}

	static pair<string, optional<string>> parse_tag(string::const_iterator& it, const string::const_iterator& end) {
		string key = parse_key(it, end);
		if (*it == '=') {
			it++;
			if (it == end)
				throw message::parsing_error("Message ended while parsing tag");
			return make_pair(key, parse_tag_value(it, end));
		} else
			return make_pair(key, nullopt);
	}

	static tags_t parse_tags(string::const_iterator& it, const string::const_iterator& end) {
		tags_t parsedTags;
		if (*it != '@') {
			return parsedTags;
		}
		do {
			it++;
			if (it == end)
				throw message::parsing_error("Message ended while parsing tags");
			parsedTags.insert(parse_tag(it, end));
		} while (*it == ';');
		if (*it != ' ')
			throw message::parsing_error("Tags do not terminate with space as they should");
		SKIP_WHITESPACES_THROW_END(it, end, "tags")
		return parsedTags;
	}

	static string parse_command(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		string command;
		if (isalpha(*it)) {
			do {
				command.push_back(*it++);
				if (it == end) {
					if (crlf_included)
						throw message::parsing_error("Message ended while parsing command");
					break;
				}
			} while (isalpha(*it));
		} else {
			if (!isdigit(*it))
				throw message::parsing_error("Message command is in wrong format");
			command.push_back(*it++);
			if (it == end) {
				throw message::parsing_error("Message ended while parsing number command");
			}
			if (!isdigit(*it))
				throw message::parsing_error("Message command is in wrong format");
			command.push_back(*it++);
			if (it == end) {
				throw message::parsing_error("Message ended while parsing number command");
			}
			if (!isdigit(*it))
				throw message::parsing_error("Message command is in wrong format");
			command.push_back(*it++);
			if (it == end) {
				if (crlf_included)
					throw message::parsing_error("Message ended while parsing number command");
			}
		}
		return command;
	}

	static string
	parse_middle_param(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		string param;
		while (*it != ' ' && *it != '\r' && *it != '\n' && *it != '\0') {
			param.push_back(*it++);
			if (it == end) {
				if (crlf_included)
					throw message::parsing_error("Message ended while parsing middle param");
				break;
			}
		}
		if (param.empty())
			throw message::parsing_error("Middle param is empty");
		return param;
	}

	static string
	parse_trailing_param(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		string param;
		while (*it != '\r' && *it != '\n' && *it != '\0') {
			param.push_back(*it++);
			if (it == end) {
				if (crlf_included)
					throw message::parsing_error("Message ended while parsing trailing param");
				break;
			}
		}
		return param;
	}

	static vector<string>
	parse_params(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		vector<string> parsedParams;
		if (it == end) {
			assert(!crlf_included);
			return parsedParams;
		}
		while (*it == ' ') {
			if (crlf_included) {
				SKIP_WHITESPACES_THROW_END(it, end, "params")
			} else {
				SKIP_WHITESPACES(it, end);
				if (it == end)
					break;
			}
			if (*it == ':') {
				it++;
				if (it == end) {
					if (crlf_included)
						throw message::parsing_error("Message ended while parsing params");
					parsedParams.emplace_back("");
					break;
				}
				parsedParams.push_back(parse_trailing_param(it, end, crlf_included));
				break;
			}
			parsedParams.push_back(parse_middle_param(it, end, crlf_included));
			if (it == end) {
				assert(!crlf_included);
				break;
			}
		}
		return parsedParams;
	}

	static optional<prefix_t> parse_prefix(string::const_iterator& it, const string::const_iterator& end) {
		if (*it != ':')
			return nullopt;
		it++; // Eat ':'
		if (it == end)
			throw message::parsing_error("Message ended while parsing prefix");
		string main;
		string user;
		optional<string> res_user;
		string host;
		optional<string> res_host;
		while (*it != ' ' && *it != '!' && *it != '@') { // Parse main
			main.push_back(*it++);
			if (it == end)
				throw message::parsing_error("Message ended while parsing prefix");
		}
		if (*it == '!') { // User part of prefix is present
			it++; // Eat '!'
			if (it == end)
				throw message::parsing_error("Message ended while parsing prefix");
			while (*it != ' ' && *it != '@') { // Parse user part
				user.push_back(*it++);
				if (it == end)
					throw message::parsing_error("Message ended while parsing prefix");
			}
			if (user.empty())
				throw message::parsing_error("Empty user part in prefix");
			res_user = make_optional(user);
		}
		if (*it == '@') { // Host part of prefix is present
			it++; // Eat '@'
			if (it == end)
				throw message::parsing_error("Message ended while parsing prefix");
			while (*it != ' ') { // Parse host part
				host.push_back(*it++); // TODO validate for valid host by rfc952
				if (it == end)
					throw message::parsing_error("Message ended while parsing prefix");
			}
			if (host.empty())
				throw message::parsing_error("Empty host part in prefix");
			res_host = make_optional(host);
		}
		if (*it != ' ')
			throw message::parsing_error("Prefix does not terminate with space as it should");
		SKIP_WHITESPACES_THROW_END(it, end, "prefix");
		return prefix_t(main, res_user, res_host);
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

message::message(const string& rawMessage, bool crlf_included) {
	auto it = rawMessage.cbegin();
	auto end = rawMessage.cend();
	if (it == end)
		throw message::parsing_error("Message is empty");
	m_tags = irc_parsing::parse_tags(it, end);
	m_prefix = irc_parsing::parse_prefix(it, end);
	m_command = irc_parsing::parse_command(it, end, crlf_included);
	m_params = irc_parsing::parse_params(it, end, crlf_included);
	if (crlf_included) {
		if (it == end)
			throw message::parsing_error("Message ends before CRLF sequence");
		if (*it++ != '\r')
			throw message::parsing_error("Expected CR character");
		if (it == end)
			throw message::parsing_error("Message ends before LF");
		if (*it++ != '\n')
			throw message::parsing_error("Expected LF character");
	}
	if (it != end)
		throw message::parsing_error("Message does not end properly");
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
		channels_param += ",#" + *c_it;
	string keys_param;
	if (!keys.empty()) {
		auto k_it = keys.cbegin();
		keys_param = *k_it;
		for (k_it++; k_it != keys.cend(); k_it++)
			keys_param += "," + *k_it;
	}
	return message({}, nullopt, "JOIN", {channels_param, keys_param});
}

message message::part_message(const vector<string>& channels) {
	string channels_param;
	assert(!channels.empty());
	auto c_it = channels.cbegin();
	channels_param = "#" + *c_it;
	for (c_it++; c_it != channels.cend(); c_it++)
		channels_param += ",#" + *c_it;
	return message({}, nullopt, "PART", {channels_param});
}

message message::pong_message(const string& daemon) {
	return message({}, nullopt, "PONG", {daemon});
}

message message::pong_message(const string& daemon1, const string& daemon2) {
	return message({}, nullopt, "PONG", {daemon1, daemon2});
}

message message::capability_request_message(const vector<string>& capabilities) {
	vector<string> params({"REQ"});
	assert(!capabilities.empty());
	auto c_it = capabilities.cbegin();
	string capabilities_str = *c_it;

	for (c_it++; c_it != capabilities.cend(); c_it++) {
		capabilities_str += ' ' + *c_it;
	}
	params.push_back(capabilities_str);
	return message({}, nullopt, "CAP", params);
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

message& message::capability_request_end_message() {
	static message message({}, nullopt, "CAP", {"END"});
	return message;
}

string prefix_t::to_irc_prefix() const {
	string prefix = m_main;
	if (m_user.has_value())
		prefix += "!" + m_user.value();
	if (m_host.has_value())
		prefix += "@" + m_host.value();
	return prefix;
}
