#include "basic_console_interface.h"

#include "../twitch_bot.h"

#include <string_view>
#include <iostream>

void basic_console_interface::show_message(const irc::message& message) {
    if (message.command() == "PRIVMSG") {
        std::string sender = twitch_bot::get_user_name_private_message(message);
        const std::string& message_text = message.params().back();
        const std::string& channel = message.params().front();
        std::cout << sender << "@" << channel << ": " << message_text << std::endl;
    }
}
