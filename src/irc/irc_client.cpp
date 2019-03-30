//
// Created by strejivo on 3/24/19.
//

#include "irc_client.h"

using std::string;
using std::cerr;
using std::endl;
using std::istringstream;
using std::chrono::milliseconds;

using irc::irc_client;
using irc::message;

using pplx::task_status::completed;
using pplx::task_status::canceled;
using pplx::task;
using pplx::create_task;
using pplx::cancellation_token;
using pplx::cancel_current_task;

using web::websockets::client::websocket_incoming_message;
using web::websockets::client::websocket_outgoing_message;
using web::websockets::client::websocket_exception;

task<void> irc_client::create_infinite_receive_task(const cancellation_token& cancellation_token) {
	return create_task(
			[=]() -> void {
				while (true) {
					if (cancellation_token.is_canceled())
						cancel_current_task();
					try {
						if (m_client.receive().then([](websocket_incoming_message msg) {
							return msg.extract_string();
						}, cancellation_token).then([=](string body) {
							istringstream is(body);
							string tmp;
							while (getline(is, tmp)) {
								tmp.pop_back();
								if (m_handle_ping) {
									try {
										message message(tmp, false);
										if (message.command() == "PING")
											send_message(message::pong_message(*message.params().begin()));
										else
											m_queued_messages.emplace(message);
									} catch (const message::parsing_error& e) {
										cerr << tmp << " : " << e.message() << endl;
										continue;
									}
								} else {
									try {
										m_queued_messages.emplace(tmp, false);
									} catch (const message::parsing_error& e) {
										cerr << tmp << " : " << e.message() << endl;
										continue;
									}
								}
							}
						}, cancellation_token).wait() == canceled)
							cancel_current_task();
					} catch (const websocket_exception& e) {
						cancel_current_task();
					}
				}
			}
	);
}

irc_client::irc_client(const string& host, bool handle_ping) : m_handle_ping(handle_ping) {
	if (m_client.connect(host).wait() != completed) {
		throw "Not connected";
	}
	m_receive_task = create_infinite_receive_task(m_cancellation_token_source.get_token());
}

task<void> irc_client::send_message(const message& message_to_send) {
	websocket_outgoing_message websocket_message;
	websocket_message.set_utf8_message(message_to_send.to_irc_message());
	return m_client.send(websocket_message);
}

message irc_client::read_message(const milliseconds& timeout) {
	auto tmp = m_queued_messages.pop(timeout);
	if (tmp.has_value())
		return tmp.value();
	throw "Timeout";
}

message irc_client::read_message() {
	return m_queued_messages.pop();
}

irc::irc_client::~irc_client() {
	m_client.close().wait();
	m_cancellation_token_source.cancel();
	assert(m_receive_task.wait() == canceled);
}
