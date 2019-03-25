//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_MESSAGE_H
#define TWITCH_IRC_MESSAGE_H


#include <string>
#include <map>
#include <vector>
#include <optional>

namespace irc {
	class message {
	  public:
		message(const std::string& rawMessage);

		message(const std::map<std::string, std::optional<std::string>>& tags, const std::string& prefix,
				const std::string& command,
				const std::vector<std::string>& params) : m_tags(tags), m_prefix(prefix), m_command(command),
														  m_params(params) {};

		static message private_message(const std::string& message_text, const std::string& channel);

		const std::string& prefix() const { return m_prefix; }

		const std::string& command() const { return m_command; }

		const std::vector<std::string>& params() const { return m_params; }

		const std::map<std::string, std::optional<std::string>>& tags() const { return m_tags; }

		std::string to_irc_message() const;

	  private:
		std::map<std::string, std::optional<std::string>> m_tags;
		std::string m_prefix;
		std::string m_command;
		std::vector<std::string> m_params;
	};
}


#endif //TWITCH_IRC_MESSAGE_H
