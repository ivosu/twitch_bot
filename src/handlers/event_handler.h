#pragma once

#include "irc/message.h"

class event_handler {
  public:
	enum event_type {
		message, subscription, room_state
	};

	enum handler_type {
		cpp, python
	};

	struct handle_arguments {
		// Might expand later
		const irc::message& event;
	};

	event_type get_handled_event_type() const { return m_event_type; }

	handler_type get_handler_type() const { return m_handler_type; }

	virtual void handle(const handle_arguments& args) = 0;

  protected:
	event_handler(event_type handled_event, handler_type handler_type) : m_event_type(handled_event),
																		 m_handler_type(handler_type) {}

	const event_type m_event_type;
	const handler_type m_handler_type;
};
