#ifndef TWITCH_IRC_PYTHON_EVENT_HANDLER_H
#define TWITCH_IRC_PYTHON_EVENT_HANDLER_H

#include "event_handler.h"

class python_event_handler : public event_handler {
  public:
	python_event_handler(event_type handled_event, const std::string& handle_code) :
			event_handler(handled_event, python), m_handle_code(handle_code) {}

	const std::string& get_handle_code() const { return m_handle_code; }

	void handle(const event_handler::handle_arguments& args) final {

	}
  private:
	const std::string m_handle_code;
};

#endif //TWITCH_IRC_PYTHON_EVENT_HANDLER_H
