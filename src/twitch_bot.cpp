#include "twitch_bot.h"

using std::string;
using std::vector;
using std::back_inserter;
using std::nullopt;
using std::optional;
using std::chrono::milliseconds;

using irc::message;
using irc::tags_t;

using pplx::task;
using pplx::task_status::completed;

string twitch_bot::get_user_name_from_user_notice_tags(const tags_t& tags) {
    auto display_name_it = tags.find("display-name");
    if (display_name_it != tags.cend() && display_name_it->second.has_value() &&
        !display_name_it->second.value().empty()) {
        return display_name_it->second.value();
    } else {
        auto login_it = tags.find("login");
        if (login_it != tags.cend() && login_it->second.has_value() && !login_it->second.value().empty()) {
            return login_it->second.value();
        } else {
            assert(false);
        }
    }
}

string twitch_bot::get_gifted_recipient_user_name(const tags_t& tags) {
    auto display_name_it = tags.find("msg-param-recipient-display-name");
    if (display_name_it != tags.end() && display_name_it->second.has_value() &&
        !display_name_it->second.value().empty()) {
        return display_name_it->second.value();
    } else {
        auto login_it = tags.find("msg-param-recipient-user-name");
        if (login_it != tags.end() && login_it->second.has_value() && !login_it->second.value().empty()) {
            return login_it->second.value();
        } else {
            assert(false);
        }
    }
}

string twitch_bot::get_user_name_private_message(const message& message) {
    assert(message.command() == "PRIVMSG");
    auto tags = message.tags();
    auto display_name_it = tags.find("display-name");
    if (display_name_it != tags.end() && display_name_it->second.has_value() &&
        !display_name_it->second.value().empty()) {
        return display_name_it->second.value();
    } else {
        assert(message.prefix().has_value());
        return message.prefix().value().main();
    }
}

twitch_bot::twitch_bot() : m_irc_client(U("wss://irc-ws.chat.twitch.tv:443"), true) {}

bool twitch_bot::login(const string& nickname, const string& auth) {
    assert(!m_logged_in);
    if (m_irc_client.send_message(message::pass_message(auth)).wait() != completed ||
        m_irc_client.send_message(message::nick_message(nickname)).wait() != completed)
        return false;
    message tmp = await_specific_command("001");
    if (!tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
        tmp.params() != vector<string>{nickname, "Welcome, GLHF!"})
        return false;
    tmp = await_specific_command("002");
    if (!tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
        tmp.params() != vector<string>{nickname, "Your host is tmi.twitch.tv"})
        return false;
    tmp = await_specific_command("003");
    if (!tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
        tmp.params() != vector<string>{nickname, "This server is rather new"})
        return false;
    tmp = await_specific_command("004");
    if (!tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
        tmp.params() != vector<string>{nickname, "-"})
        return false;
    tmp = await_specific_command("375");
    if (!tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
        tmp.params() != vector<string>{nickname, "-"})
        return false;
    tmp = await_specific_command("372");
    if (!tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
        tmp.params() != vector<string>{nickname, "You are in a maze of twisty passages, all alike."})
        return false;
    tmp = await_specific_command("376");
    if (!tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv" ||
        tmp.params() != vector<string>{nickname, ">"})
        return false;
    tmp = await_specific_command("GLOBALUSERSTATE");
    m_logged_in = true;
    m_nickname = nickname;
    return true;
}

task<void> twitch_bot::send_message(const string& message, const string& channel) {
    assert(m_logged_in);
    assert(m_joined_channels.find(channel) != m_joined_channels.end());
    return m_irc_client.send_message(message::private_message(message, channel));
}

message twitch_bot::read_message(const milliseconds& timeout) {
    assert(m_logged_in);
    if (m_handle_later_messages.empty())
        return m_irc_client.read_message(timeout);

    message tmp = m_handle_later_messages.front();
    m_handle_later_messages.pop_front();
    return tmp;
}

message twitch_bot::read_message() {
    assert(m_logged_in);
    if (m_handle_later_messages.empty())
        return m_irc_client.read_message();

    message tmp = m_handle_later_messages.front();
    m_handle_later_messages.pop_front();
    return tmp;
}

bool twitch_bot::part_channel(const string& channel) {
    assert(m_logged_in);
    assert(m_joined_channels.find(channel) != m_joined_channels.end());
    if (m_irc_client.send_message(message::part_message({channel})).wait() == completed) {
        if (await_specific_command("PART").params() != vector<string>{"#" + channel})
            return false;
        m_joined_channels.erase(channel);
        return true;
    }
    return false;
}

bool twitch_bot::join_channel(const string& channel) {
    assert(m_logged_in);
    if (m_joined_channels.find(channel) != m_joined_channels.end())
        return true;
    if (m_irc_client.send_message(message::join_message({channel})).wait() != completed)
        return false;

    message tmp = await_specific_command("JOIN");
    if (tmp.params() != vector<string>{"#" + channel} ||
        !tmp.prefix().has_value() || tmp.prefix().value().to_irc_prefix() !=
                                     m_nickname + "!" + m_nickname + "@" + m_nickname + ".tmi.twitch.tv")
        return false;

    tmp = await_specific_command("353");
    if (tmp.params() != vector<string>{m_nickname, "=", "#" + channel, m_nickname} ||
        !tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
        return false;

    tmp = await_specific_command("366");
    if (tmp.params() != vector<string>{m_nickname, "#" + channel, "End of /NAMES list"} ||
        !tmp.prefix().has_value() ||
        tmp.prefix().value().to_irc_prefix() != m_nickname + ".tmi.twitch.tv")
        return false;

    tmp = await_specific_command("USERSTATE");
    if (tmp.params() != vector<string>{"#" + channel} ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv")
        return false;

    tmp = await_specific_command("ROOMSTATE");
    if (tmp.params() != vector<string>{"#" + channel} ||
        tmp.prefix().value().to_irc_prefix() != "tmi.twitch.tv")
        return false;

    m_joined_channels.insert(channel);
    return true;
}

bool twitch_bot::cap_req(const vector<string>& capabilities) {
    m_irc_client.send_message(message::capability_request_message(capabilities)).wait();
    vector<string> expected_ack_params = vector<string>{"*", "ACK"};
    auto c_it = capabilities.cbegin();
    string capabilities_str = *c_it;

    for (c_it++; c_it != capabilities.cend(); c_it++) {
        capabilities_str += ' ' + *c_it;
    }
    expected_ack_params.push_back(capabilities_str);

    if (await_specific_command("CAP").params() != expected_ack_params)
        return false;

    return m_irc_client.send_message(message::capability_request_end_message()).wait() == completed;
}

message twitch_bot::await_specific_command(const string& command) {
    for (auto it = m_handle_later_messages.begin(); it != m_handle_later_messages.end(); it++) {
        if (it->command() == command) {
            message tmp = *it;
            m_handle_later_messages.erase(it);
            return tmp;
        }
    }
    while (true) {
        message tmp = m_irc_client.read_message();
        if (tmp.command() == command)
            return tmp;
        m_handle_later_messages.push_back(tmp);
    }
}

message twitch_bot::await_specific_command(const string& command, const milliseconds& timeout) {
    for (auto it = m_handle_later_messages.begin(); it != m_handle_later_messages.end(); it++) {
        if (it->command() == command) {
            message tmp = *it;
            m_handle_later_messages.erase(it);
            return tmp;
        }
    }
    while (true) {
        message tmp = m_irc_client.read_message(timeout);
        if (tmp.command() == command)
            return tmp;
        m_handle_later_messages.push_back(tmp);
    }
}
