#ifndef TWITCH_IRC_EVENT_HANDLER_H
#define TWITCH_IRC_EVENT_HANDLER_H


#include "../irc/message.h"

class event_handler {
  public:
	enum event_type {
		message, subscription, room_state
	};
	static const std::string& event_type_to_string (event_type){}
	static event_type event_type_from_string(const std::string& even_type_str){}

	enum handler_type {
		cpp, python

	};
	static const std::string& handler_type_to_string(handler_type){}
	static handler_type handler_type_from_string(const std::string& handler_type_str){}

	struct handle_arguments {
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


#endif //TWITCH_IRC_EVENT_HANDLER_H
