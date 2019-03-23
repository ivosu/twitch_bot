//
// Created by strejivo on 3/22/19.
//

#include "OutcomingMessage.h"

using std::string;
using std::variant;
using std::vector;
using std::map;

OutcomingMessage::OutcomingMessage(const map<string, variant<string, void>>& tags, const string& prefix,
								   const string& command, const vector<string>& params) {
	m_Tags = tags;
	m_Prefix = prefix;
	m_Command = command;
	m_Params = params;
}