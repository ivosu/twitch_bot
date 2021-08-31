#include <iostream>
#include <libconfig.h++>
#include <mongocxx/instance.hpp>
#include "src/twitch_bot.h"
#include "src/db/mongodb_communicator.h"
#include "src/utils/temporary_file.hpp"
#include "src/db/support/configuration/mongodb_libconfig.h"

int main() {
    mongocxx::instance instance{};

    libconfig::Config conf;
    try {
        conf.readFile("config");
    } catch (libconfig::FileIOException& e) {
        std::cerr << "Config file not found!" << std::endl;
    } catch (libconfig::ParseException& e) {
        std::cerr << e.getError() << " on line " << e.getLine() << std::endl;
    }
    twitch_bot bot;
    libconfig::Setting& login = conf.lookup("twitch.login");
    std::string username;
    std::string auth;
    if (!login.lookupValue("username", username) || !login.lookupValue("auth", auth)) {
        std::cerr << "Failed to load login info" << std::endl;
        return 1;
    }

    std::vector<std::string> channels = {"ivosu", "agraelus"};

    bot.cap_req({"twitch.tv/commands", "twitch.tv/tags"});

    if (!bot.login(username, auth)) {
        std::cerr << "Failed to login" << std::endl;
        return 1;
    }

    for (const std::string& channel: channels) {
        if (!bot.join_channel(channel)) {
            std::cerr << "Failed to join channel " << channel << std::endl;
            return 1;
        }
    }

    std::cerr << "Chat connected" << std::endl;
    mongodb_communicator com(mongodb_libconfig(conf.lookup("mongodb")));
    while (true) {
        irc::message tmp = bot.read_message();
        if (!com.save_message(tmp)) {
            std::cerr << "failed to save message" << std::endl;
        }
        if (tmp.command() == "PRIVMSG") {
            std::string sender = twitch_bot::get_user_name_private_message(tmp);
            std::string message = *tmp.params().rbegin();
            std::string channel = tmp.params().front();
            channel.erase(channel.begin());
            if ((sender == "Ivosu" || sender == "ivosu") && message == "!stop")
                break;
            std::cout << sender << "@" << channel << ": " << message << std::endl;
        }
    }
    return 0;
}
