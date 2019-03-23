//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_MESSAGE_H
#define TWITCH_IRC_MESSAGE_H


#include <string>
#include <map>
#include <vector>
#include <optional>

class Message {
  public:
	const std::string& getPrefix() const;

	const std::string& getCommand() const;

	const std::vector<std::string>& getParams() const;

	const std::map<std::string, std::optional<std::string>>& getTags() const;

  protected:
	Message() = default;

	std::string m_Prefix;
	std::string m_Command;
	std::vector<std::string> m_Params;
	std::map<std::string, std::optional<std::string>> m_Tags;
};


#endif //TWITCH_IRC_MESSAGE_H
