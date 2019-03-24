//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_MESSAGE_H
#define TWITCH_IRC_MESSAGE_H


#include <string>
#include <map>
#include <vector>
#include <optional>

using std::string;
using std::vector;
using std::map;
using std::optional;

class Message {
  public:
	Message(const string& rawMessage);

	Message(const map<string, optional<string>>& tags, const string& prefix, const string& command,
			const vector<string>& params) : m_Tags(tags), m_Prefix(prefix), m_Command(command), m_Params(params){};

	const string& getPrefix() const;

	const string& getCommand() const;

	const vector<string>& getParams() const;

	const map<string, optional<string>>& getTags() const;

	string toIRCMessage() const;

  private:
	map<string, optional<string>> m_Tags;
	string m_Prefix;
	string m_Command;
	vector<string> m_Params;
};


#endif //TWITCH_IRC_MESSAGE_H
