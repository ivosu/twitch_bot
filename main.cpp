#include <iostream>
#include <regex>
#include <libconfig.h++>
#include <cpprest/ws_client.h>
#include "src/TwitchBot.h"

int main() {
	libconfig::Config conf;
	try {
		conf.readFile("config");
	} catch (libconfig::FileIOException& e) {
		std::cerr << "Config file not found!" << std::endl;
	} catch (libconfig::ParseException& e) {
		std::cerr << e.getError() << " on line " << e.getLine() << std::endl;
	}
	TwitchBot bot;
	libconfig::Setting& login = conf.lookup("login");
	std::string username;
	std::string auth;
	if (!login.lookupValue("username", username) || !login.lookupValue("auth", auth)) {
		std::cerr << "Failed to load login info" << std::endl;
		return 1;
	}
	//std::string channel = "fattypillow";
	std::string channel = "ivosu";
	if (!bot.connect()) {
		std::cerr << "Connection failed" << std::endl;
		return 1;
	}
	if (!bot.login(username, auth)) {
		std::cerr << "Failed to login" << std::endl;
		return 1;
	}
	if (!bot.joinChannel(channel)) {
		std::cerr << "Failed to join channel " << channel << std::endl;
		return 1;
	}
	if (!bot.capReq("twitch.tv/commands twitch.tv/tags")) {
		std::cout << "Unable to request command and tags" << std::endl;
	}
	auto tmp = bot.readMessage();
	while (tmp.first) {
		/*const std::string& sender = tmp.second->m_Sender.getNickname();
		const std::string& message = tmp.second->m_Message;
		std::cout<<sender<<":"<<message<<std::endl;
		*/
		/*if (message.size() > 1 && message[0] == '!') {
		   auto spacePos = message.find(' ');
		   bot.onCommand(sender, message.substr(1, spacePos - 1),
						 (spacePos != std::string::npos ? message.substr(message.find(' ') + 1) : ""));
	   }*/
		tmp = bot.readMessage();
	}
	return 0;
}
