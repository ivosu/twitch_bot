#include <iostream>
#include <regex>
#include <libconfig.h++>
#include <cpprest/ws_client.h>
#include <cpprest/ws_client.h>

using namespace web;
using namespace web::websockets::client;

struct ChatMessage {
	ChatMessage(const std::string& sender, const std::string & message) : m_Sender(sender), m_Message(message) {}

	const std::string m_Sender, m_Message;
};

class TwitchBot {
  public:
	bool connect() {
		try {
			return m_Client.connect(U("wss://irc-ws.chat.twitch.tv:443")).wait() == pplx::task_status::completed;
		} catch (...) {
			return false;
		}
	}

	bool login(const std::string& nickname, const std::string& auth, ...) {
		assert(!m_LoggedIn);
		if (sendSimpleMessage("PASS " + auth).wait() != pplx::task_status::completed || sendSimpleMessage("NICK " + nickname).wait() != pplx::task_status::completed)
			return false;
		auto ret = readSimpleMessage();
		if (!ret.first)
			return false;
		if (ret.second == ":tmi.twitch.tv 001 " + nickname + " :Welcome, GLHF!\r\n"
															 ":tmi.twitch.tv 002 " + nickname +
						  " :Your host is tmi.twitch.tv\r\n"
						  ":tmi.twitch.tv 003 " + nickname + " :This server is rather new\r\n"
															 ":tmi.twitch.tv 004 " + nickname + " :-\r\n"
																								":tmi.twitch.tv 375 " +
						  nickname + " :-\r\n"
									 ":tmi.twitch.tv 372 " + nickname +
						  " :You are in a maze of twisty passages, all alike.\r\n"
						  ":tmi.twitch.tv 376 " + nickname + " :>\r\n") {
			m_LoggedIn = true;
			m_Nickname = nickname;
			return true;
		} else return false;
	}

	pplx::task<void> sendMessage(const std::string& message, const std::string& channel) {
		assert(m_LoggedIn);
		return sendSimpleMessage("PRIVMSG #"+channel+" :"+message);
	}

	pplx::task<void> sendMessage(const std::string& message) {
		assert(m_LoggedIn);
		assert(m_JoinedChannel);
		return sendMessage(message, m_Channel);
	}

	std::pair<bool, std::unique_ptr<ChatMessage>> readMessage(unsigned int timeout = 0) {
		assert(m_LoggedIn);
		assert(m_JoinedChannel);
		auto tmp = readSimpleMessage();
		if (!tmp.first)
			return std::make_pair(false, nullptr);
		if (tmp.second == "PING :tmi.twitch.tv\r\n") {
			std::cout<<"PING!"<<std::endl;
			sendSimpleMessage("PONG :tmi.twitch.tv");
			return readMessage(timeout);
		}
		std::regex messageRegex(R"(:([a-zA-Z0-9_]{4,25})!([a-zA-Z0-9_]{4,25})@([a-zA-Z0-9_]{4,25})\.tmi\.twitch\.tv PRIVMSG #([a-zA-Z0-9_]{4,25}) :(.*)\r\n)");
		std::smatch match;
		if (!std::regex_match(tmp.second, match, messageRegex) || match[1] != match[2] || match[1] != match[3]) {
			std::cerr<<"Malformated message?"<<std::endl<<tmp.second<<std::endl;
			return std::make_pair(false, nullptr);
		}
		std::string username = match[1], message = match[5];
		if (match[4] != m_Channel) return readMessage(timeout);
		return std::make_pair(true, std::move(std::make_unique<ChatMessage>(username, message)));
	}

	bool partChannel() {
		assert(m_LoggedIn);
		assert(m_JoinedChannel);
		if (sendSimpleMessage("PART  #"+m_Channel).wait() != pplx::task_status::completed)
			return false;
		m_JoinedChannel = false;
		return true;
	}

	bool joinChannel(const std::string& channel) {
		assert(m_LoggedIn);
		if (m_JoinedChannel)
			partChannel();
		if (sendSimpleMessage("JOIN #" + channel).wait() != pplx::task_status::completed)
			return false;
		auto tmp = readSimpleMessage();
		if (!tmp.first)
			return false;
		if (tmp.second !=
			":" + m_Nickname + "!" + m_Nickname + "@" + m_Nickname + ".tmi.twitch.tv JOIN #" + channel + "\r\n") {
			return false;
		}
		tmp = readSimpleMessage();
		if (!tmp.first)
			return false;
		if (tmp.second ==
			   ":" + m_Nickname + ".tmi.twitch.tv 353 " + m_Nickname + " = #" + channel + " :" + m_Nickname + "\r\n"
																											  ":" +
			   m_Nickname + ".tmi.twitch.tv 366 " + m_Nickname + " #" + channel + " :End of /NAMES list\r\n"){
			m_JoinedChannel = true;
			m_Channel = channel;
			return true;
		} return false;
	}

	void onCommand(const std::string& sender, const std::string& command, const std::string& restOfCommand) {
		if (command == "hello")
			pplx::task<void>([this, sender]{
				sendMessage("Hello " + sender);
			});
	}
	bool capReq(std::string stuff) {// TODO refactor
		sendSimpleMessage("CAP REQ :"+stuff).wait();
		auto tmp = readSimpleMessage();
		if (!tmp.first)
			return false;
		return tmp.second == ":tmi.twitch.tv CAP * ACK :" + stuff + "\r\n";
	}
  	private:
	std::pair<bool, std::string> readSimpleMessage() {
		std::string ret;
		try {
			if (m_Client.receive().then([](websocket_incoming_message msg) {
				return msg.extract_string();
			}).then([&ret](std::string body) {
				ret = body;
			}).wait() == pplx::task_status::completed) {
				return std::make_pair(true, ret);
			} else return std::make_pair(false, "");
		} catch (...) {
			return std::make_pair(false, "");
		}
	}

	 pplx::task<void> sendSimpleMessage(const std::string& message) {
		websocket_outgoing_message msg;
		msg.set_utf8_message(message);
		return m_Client.send(msg);
	}

	websocket_client m_Client;
	std::string m_Nickname;
	bool m_LoggedIn = false;
	bool m_JoinedChannel = false;
	std::string m_Channel;
};

int main() {
	libconfig::Config conf;
	try{
		conf.readFile("config");
	} catch (libconfig::FileIOException& e){
		std::cerr<<"Config file not found!"<<std::endl;
	} catch (libconfig::ParseException& e){
		std::cerr<<e.getError()<<" on line "<<e.getLine()<<std::endl;
	}
	TwitchBot bot;
	libconfig::Setting& login = conf.lookup("login");
	std::string username;
	std::string auth;
	if (!login.lookupValue("username", username) || !login.lookupValue("auth", auth)) {
		std::cerr << "Failed to load login info" << std::endl;
		return 1;
	}
	std::string channel = "fattypillow";
	if (!bot.connect()) {
		std::cerr << "Connection failed" << std::endl;
		return 1;
	}
	if (!bot.login(username, auth)) {
		std::cerr << "Failed to login" << std::endl;
		return 1;
	}
	if (!bot.joinChannel(channel)) {
		std::cerr << "Failed to join channel "<< channel << std::endl;
		return 1;
	}
	if (!bot.capReq("twitch.tv/commands twitch.tv/tags")) {
		std::cout<<"Unable to request command and tags"<<std::endl;
	}
	auto tmp = bot.readMessage();
	while(tmp.first){
		const std::string& sender = tmp.second->m_Sender;
		const std::string& message = tmp.second->m_Message;
		std::cout<<sender<<":"<<message<<std::endl;
		/*if (message.size() > 1 && message[0] == '!') {
			auto spacePos = message.find(' ');
			bot.onCommand(sender, message.substr(1, spacePos - 1),
						  (spacePos != std::string::npos ? message.substr(message.find(' ') + 1) : ""));
		}*/
		tmp = bot.readMessage();
	}
	return 0;
}
