#pragma once

#include "irc/message.h"
#include "irc/client.h"
#include <string>
#include <set>
#include <list>

class twitch_bot {
public:

    twitch_bot();

    bool login(const std::string& nickname, const std::string& auth);

    pplx::task<void> send_message(const std::string& message, const std::string& channel);

    irc::message read_message(const std::chrono::milliseconds& timeout);

    irc::message read_message();

    irc::message await_specific_command(const std::string& command, const std::chrono::milliseconds& timeout);

    irc::message await_specific_command(const std::string& command);

    bool part_channel(const std::string& channel);

    bool join_channel(const std::string& channel);

    bool cap_req(const std::vector<std::string>& capabilities);

    static std::string
    get_user_name_from_user_notice_tags(const irc::tags_t& tags);

    static std::string get_user_name_private_message(const irc::message& message);

    static std::string get_gifted_recipient_user_name(const irc::tags_t& tags);

private:
    irc::client m_irc_client;
    std::string m_nickname;
    bool m_logged_in = false;
    std::set<std::string> m_joined_channels;
    std::list<irc::message> m_handle_later_messages;
};
