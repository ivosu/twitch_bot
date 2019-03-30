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
	//std::string channel = "ivosu";
	std::string channel = "twitchpresents";

	bot.cap_req({"twitch.tv/commands", "twitch.tv/tags"});

	if (!bot.login(username, auth)) {
		std::cerr << "Failed to login" << std::endl;
		return 1;
	}


	/*if (!bot.join_channel(channel)) {
		std::cerr << "Failed to join channel " << channel << std::endl;
		return 1;
	}
	if (!bot.join_channel("ivosu")) {
		std::cerr << "Failed to join channel ivosu" << std::endl;
		return 1;
	}*/
	if (!bot.join_channel("starladder_cs_en")) {
		std::cerr << "Failed to join channel starladder_cs_en"<<std::endl;
		return 1;
	}
	/*if (!bot.join_channel("lck_korea")) {
		std::cerr << "Failed to join channel lck_korea" << std::endl;
		return 1;
	}*/

	std::cout << "Chat connected" << std::endl;
	while (true) {
		irc::message tmp = bot.read_message();
		if (tmp.command() == "PRIVMSG") {
			std::string sender = twitch_bot::get_user_name_private_message(tmp);
			std::string message = *tmp.params().rbegin();
			//std::cout << sender << " : " << message << std::endl;
			if ((sender == "Ivosu" || sender == "ivosu") && message == "!stop")
				break;
		} else {
			//std::cout << tmp.to_irc_message();
			/*if (tmp.command() == "USERNOTICE") {
				auto tags = tmp.tags();
				auto msg_id_it = tags.find("msg-id");
				if (msg_id_it == tags.cend() || !msg_id_it->second.has_value())
					continue;
				const std::string& msg_id = msg_id_it->second.value();
				if (msg_id == "sub" || msg_id == "resub") {
					std::string user = twitch_bot::get_user_name_from_user_notice_tags(tags);
					bot.send_message(
					  "fattySub fattySub fattySub Vítej " + std::string(msg_id == "resub" ? "zpátky " : "") + user +
					  " do naší tučné rodiny fattySub fattySub fattySub", channel);
				} else if (msg_id == "subgift" || msg_id == "anonsubgift") {
					std::string gifter = twitch_bot::get_user_name_from_user_notice_tags(tags);
					std::string gifted = twitch_bot::get_gifted_recipient_user_name(tags);
					bot.send_message(
					  "fattySub fattySub fattySub Vítej " + gifted + " do naší tučné rodiny fattySub fattySub fattySub",
					  channel);
				}
			}*/
		}
	}
	return 0;
}
