//
// Created by strejivo on 3/24/19.
//

#include "IRCClient.h"

using std::istringstream;

IRCClient::IRCClient(std::string host) {
	if (m_Client.connect(host).wait() != pplx::task_status::completed) {
		throw "Not connected";
	}
}

pplx::task<void> IRCClient::sendMessage(const Message& message) {
	web::web_sockets::client::websocket_outgoing_message msg;
	msg.set_utf8_message(message.toIRCMessage());
	return m_Client.send(msg);
}

Message IRCClient::readMessage(size_t timeout) {
	if (m_QueuedMessaged.empty()) {
		string in;
		if (m_Client.receive().then([](web::web_sockets::client::websocket_incoming_message msg) {
			return msg.extract_string();
		}).then([&in](std::string body) { in = body; }).wait() == pplx::task_status::completed) {
			istringstream is(in);
			string tmp;
			while(getline(is, tmp)) {
				tmp += "\n"; // Put back the newline so the parsing is proper
				m_QueuedMessaged.emplace(tmp);
			}
		}
	}
	if (!m_QueuedMessaged.empty()) {
		Message front = m_QueuedMessaged.front();
		m_QueuedMessaged.pop();
		return front;
	} else throw "No message received";
}
