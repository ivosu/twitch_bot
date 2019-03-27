//
// Created by strejivo on 3/22/19.
//

#ifndef TWITCH_IRC_MESSAGE_H
#define TWITCH_IRC_MESSAGE_H


#include <string>
#include <map>
#include <vector>
#include <optional>
#include <memory>

namespace irc {
	namespace prefix {
		class prefix {
		  public:
			prefix(const std::string& main, const std::optional<std::string>& user,
				   const std::optional<std::string>& host) : m_main(main), m_user(user), m_host(host) {}

			const std::string& main() const { return m_main; }

			const std::optional<std::string>& user() const { return m_user; }

			const std::optional<std::string>& host() const { return m_host; }

			std::string to_irc_prefix() const;

		  private:
			std::string m_main;
			std::optional<std::string> m_user;
			std::optional<std::string> m_host;
		};
	}

	class message {
	  public:
		message(const std::string& rawMessage);

		message(const std::map<std::string, std::optional<std::string>>& tags,
				const std::optional<prefix::prefix>& prefix,
				const std::string& command,
				const std::vector<std::string>& params) : m_tags(tags), m_prefix(prefix), m_command(command),
														  m_params(params) {};

		static message private_message(const std::string& message_text, const std::string& channel);

		static message pass_message(const std::string& password);

		static message nick_message(const std::string& nickname);

		static message
		join_message(const std::vector<std::string>& channels, const std::vector<std::string>& keys = {});

		static message part_message(const std::vector<std::string>& channels);

		static message pong_message(const std::string& daemon);

		static message pong_message(const std::string& daemon1, const std::string& deamon2);

		static message capability_request_message(const std::vector<std::string>& capabilities);

		const std::optional<prefix::prefix>& prefix() const { return m_prefix; }

		const std::string& command() const { return m_command; }

		const std::vector<std::string>& params() const { return m_params; }

		const std::map<std::string, std::optional<std::string>>& tags() const { return m_tags; }

		std::string to_irc_message() const;

	  private:
		std::map<std::string, std::optional<std::string>> m_tags;
		std::optional<irc::prefix::prefix> m_prefix;
		std::string m_command;
		std::vector<std::string> m_params;
	};
}


#endif //TWITCH_IRC_MESSAGE_H
