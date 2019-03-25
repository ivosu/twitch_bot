//
// Created by strejivo on 3/24/19.
//

#include "irc_client.h"

using std::string;
using std::istringstream;

irc::irc_client::irc_client(std::string host) {
	if (m_client.connect(host).wait() != pplx::task_status::completed) {
		throw "Not connected";
	}
}

pplx::task<void> irc::irc_client::send_message(const irc::message& message_to_send) {
	web::web_sockets::client::websocket_outgoing_message msg;
	msg.set_utf8_message(message_to_send.to_irc_message());
	return m_client.send(msg);
}

irc::message irc::irc_client::read_message(size_t timeout) {
	if (m_queued_messaged.empty()) {
		string in;
		if (m_client.receive().then([](web::web_sockets::client::websocket_incoming_message msg) {
			return msg.extract_string();
		}).then([&in](std::string body) { in = body; }).wait() == pplx::task_status::completed) {
			istringstream is(in);
			string tmp;
			while (getline(is, tmp)) {
				tmp += "\n"; // Put back the newline so the parsing is proper
				m_queued_messaged.emplace(tmp);
			}
		}
	}
	if (!m_queued_messaged.empty()) {
		irc::message front = m_queued_messaged.front();
		m_queued_messaged.pop();
		return front;
	} else throw "No message received";
}
