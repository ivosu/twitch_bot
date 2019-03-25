#include <iostream>
#include <libconfig.h++>
#include "src/twitch_bot.h"

int main() {
	libconfig::Config conf;
	try {
		conf.readFile("config");
	} catch (libconfig::FileIOException& e) {
		std::cerr << "Config file not found!" << std::endl;
	} catch (libconfig::ParseException& e) {
		std::cerr << e.getError() << " on line " << e.getLine() << std::endl;
	}
	twitch_bot bot;
	libconfig::Setting& login = conf.lookup("login");
	std::string username;
	std::string auth;
	if (!login.lookupValue("username", username) || !login.lookupValue("auth", auth)) {
		std::cerr << "Failed to load login info" << std::endl;
		return 1;
	}
	//std::string channel = "fattypillow";
	std::string channel = "ivosu";

	if (!bot.login(username, auth)) {
		std::cerr << "Failed to login" << std::endl;
		return 1;
	}
	if (!bot.join_channel(channel)) {
		std::cerr << "Failed to join channel " << channel << std::endl;
		return 1;
	}
	if (!bot.cap_req("twitch.tv/commands twitch.tv/tags")) {
		std::cerr << "Unable to request command and tags" << std::endl;
	}
	while (true) {
		irc::message tmp = bot.read_message();
		if (tmp.command() == "PRIVMSG") {
			std::string sender;
			auto tags = tmp.tags();
			if (tags["display-name"].has_value()) {
				sender = tags["display-name"].value();
			}
			else {
				for (const auto& c : tmp.prefix()) {
					if (c != '!')
						sender.push_back(c);
					else break;
				}
			}
			std::string message = *tmp.params().rbegin();
			std::cout<<sender<<" : "<<message<<std::endl;
			if ((sender == "Ivosu" || sender == "ivosu") && message == "!stop")
				break;
		} else std::cout<< tmp.to_irc_message();
	}
	return 0;
}
