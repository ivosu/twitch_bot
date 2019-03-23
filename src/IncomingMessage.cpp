//
// Created by strejivo on 3/22/19.
//

#include "IncomingMessage.h"
#include <regex>
#include <assert.h>

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::regex;
using std::smatch;
using std::optional;
using std::make_pair;
using std::nullopt;

string getUnescapedTagValue(const string& escapedTagValue) {
	string unescapedTagValue;
	for (auto it = escapedTagValue.begin(); it < escapedTagValue.end(); it++) {
		switch (*it) {
			case '\\': {
				if (++it == escapedTagValue.end()) {
					break;
				} else {
					switch (*it) {
						case ':':
							unescapedTagValue.push_back(';');
							break;
						case 's':
							unescapedTagValue.push_back(' ');
							break;
						case '\\':
							unescapedTagValue.push_back('\\');
							break;
						case 'r':
							unescapedTagValue.push_back('\r');
							break;
						case 'n':
							unescapedTagValue.push_back('\n');
							break;
						default:
							throw "Unknown escape sequence"; // TODO
					}
				}
				break;
			}
			default:
				unescapedTagValue.push_back(*it);
		}
	}
	return unescapedTagValue;
}

string parseTagValue(string::const_iterator& it, const string::iterator& end) {
	string tagValue;
	while(it != end && *it != ';' && *it != ' ' && *it != '\r' && *it != '\n' && *it != '\0') {
		//if
	}
}

string parseKey(string::const_iterator& it, const string::iterator& end) {
	string key;
	while(it != end && (isalnum(*it) || *it == '-' || *it == '/' || *it == '+')) // TODO make precisely as in rfc1459
		key.push_back(*(it++));
	assert(!key.empty());
	return key;
}

pair<string, optional<string>> parseTag (string::const_iterator& it, const string::iterator& end) {
	string key = parseKey(it, end);
	if (it != end && *it == '=')
		return make_pair(key, parseTagValue(it, end));
	else
		return make_pair(key, nullopt);
}

map<string, optional<string>> parseTags(string::const_iterator& it, const string::iterator& end) {
	map<string, optional<string>> parsedTags;
	if (it == rawMessage.end() || *it != '@') {
		return parsedTags;
	}
	it++;
	par
	return parsedTags;
}

vector<string> parseParams(const string& params) {
	vector<string> parsedParams;
	for (auto it = params.begin(); it != params.end(); it++) {
		assert(*it == ' ');
		do {
			it++;
		} while (it != params.end() && *it == ' ');
		if (it == params.end())
			throw ""; // TODO
		string param;
		if (*it == ':') {
			// We can just copy all (trailing part)
			param.insert(params.begin(), ++it, params.end());
			it = params.end();
		} else {
			// Check for next space (middle)
			while (it != param.end() && *it != ' ') {
				param.push_back(*(it++));
			}
		}
		assert(!param.empty());
		parsedParams.push_back(param);
	}
	return parsedParams;
}

IncomingMessage::IncomingMessage(const string& rawMessage) {
	static regex messageRegex(
			R"(^(?:@([^ ]+)[ ]+)?(?::([^ ]+)[ ]+)?([a-zA-Z]+|[0-9]{3})([ ][^\r\n ][^\r\n]*)\r\n)");
	smatch match;
	if (!regex_match(rawMessage, match, messageRegex)) {
		throw "Unknown message " + rawMessage;
	}
	m_Tags = parseTags(match[1]);
	m_Prefix = match[2];
	m_Command = match[3];
	m_Params = parseParams(match[4]);
}
