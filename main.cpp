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

	bot.cap_req({"twitch.tv/commands", "twitch.tv/tags"});

	std::cout<<"Chat connected"<<std::endl;
	while (true) {
		irc::message tmp = bot.read_message();
		if (tmp.command() == "PRIVMSG") {
			std::string sender;
			try{
				auto tags = tmp.tags();
				if (tags.at("display-name").has_value()) {
					sender = tags["display-name"].value();
				} else {
					assert(tmp.prefix().has_value());
					sender = tmp.prefix().value().main();
				}
			} catch (const std::out_of_range& e) {
				assert(tmp.prefix().has_value());
				sender = tmp.prefix().value().main();
			}
			std::string message = *tmp.params().rbegin();
			std::cout << sender << " : " << message << std::endl;
			if ((sender == "Ivosu" || sender == "ivosu") && message == "!stop")
				break;
		} else {
			std::cout << tmp.to_irc_message();
			if (tmp.command() == "USERNOTICE") {
				auto tags = tmp.tags();
				auto msg_id_it = tags.find("msg-id");
				if (msg_id_it == tags.cend() || !msg_id_it->second.has_value())
					continue;
				const std::string& msg_id = msg_id_it->second.value();
				if (msg_id == "sub" || msg_id == "resub") {
					std::string user;
					auto display_name_it = tags.find("display-name");
					if (display_name_it != tags.cend() && display_name_it->second.has_value() && !display_name_it->second.value().empty()) {
						user = display_name_it->second.value();
					} else {
						auto login_it = tags.find("login");
						if (login_it != tags.cend() && login_it->second.has_value() && !login_it->second.value().empty()) {
							user = login_it->second.value();
						} else {
							// DAFAK?
							continue;
						}
					}
					bot.send_message("fattySub fattySub fattySub Vítej " + std::string(msg_id == "resub" ? "zpátky " : "") + user +
									 " do naší tučné rodiny fattySub fattySub fattySub", channel);
				}
				/*
				try {
					if (tags.at("msg-id").has_value()) {
						if (tags.at("msg-id").value() == "sub") {
							std::string user;
							try {
								if (tags.at("display-name").has_value() && !tags.at("display-name").value().empty()) {
									user = tags.at("display-name").value();
								} else {
									try {
										user = tags.at("login").value();
									} catch (const std::out_of_range& e) {
										// No nick????
									}
								}
							} catch (const std::out_of_range& e) {

							}
						}
					}
				} catch (const std::out_of_range& e) {

				}

						} else
						bot.send_message(
						  "fattySub fattySub fattySub Vítej " + user + " do tučné rodiny fattySub fattySub fattySub",
						  channel);
					} else if (tags.at("msg-id").value() == "resub") {
						std::string user;
						if (tags.at("display-name").has_value() && !tags.at("display-name").value().empty()) {
							user = tags.at("display-name").value();
						} else user = tags.at("login").value();

					}
				}*/
			}
		}
	}
	return 0;
}
