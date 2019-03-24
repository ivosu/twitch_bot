#include <iostream>
#include <regex>
#include <libconfig.h++>
#include <cpprest/ws_client.h>
#include "src/TwitchBot.h"
#include "src/irc/Message.h"

int main() {
	/*string exampleMessage = "@badges=subscriber/12,sub-gifter/1;color=;display-name=staxcz;emotes=3:25-26;flags=;id=9bbbf1e1-31e4-4243-a882-dd69b83e6bf4;mod=0;room-id=76073513;subscriber=1;tmi-sent-ts=1553120470096;turbo=0;user-id=107979616;user-type= :staxcz!staxcz@staxcz.tmi.twitch.tv PRIVMSG #fattypillow :na ptaka sou top zaclony :D\r\n";
	Message a(exampleMessage);
	std::cout<<exampleMessage<<std::endl<<a.toIRCMessage()<<std::endl;
	assert(a.toIRCMessage() == exampleMessage);
	string exampleMessage2 = "JOIN #blabla\r\n";
	Message b(exampleMessage2);
	std::cout<<exampleMessage2<<std::endl<<b.toIRCMessage()<<std::endl;
	assert(b.toIRCMessage() == exampleMessage2);
	return 0;*/
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
	std::string channel = "twitchpresents";

	if (!bot.login(username, auth)) {
		std::cerr << "Failed to login" << std::endl;
		return 1;
	}
	if (!bot.joinChannel(channel)) {
		std::cerr << "Failed to join channel " << channel << std::endl;
		return 1;
	}
	if (!bot.capReq("twitch.tv/commands twitch.tv/tags")) {
		std::cerr << "Unable to request command and tags" << std::endl;
	}
	while (true) {
		Message tmp = bot.readMessage();
		std::cout<<tmp.toIRCMessage()<<std::endl;
	}
	return 0;
}
