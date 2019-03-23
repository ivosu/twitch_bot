//
// Created by strejivo on 3/22/19.
//

#include "Message.h"

using std::string;
using std::map;
using std::vector;
using std::variant;

const string& Message::getPrefix() const {
	return m_Prefix;
}

const string& Message::getCommand() const {
	return m_Command;
}

const vector<string>& Message::getParams() const {
	return m_Params;
}

const map<string, variant<string, void>>& Message::getTags() const {
	return m_Tags;
}

