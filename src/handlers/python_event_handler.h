#pragma once

#include "event_handler.h"

#include <utility>

class python_event_handler : public event_handler {
  public:
	python_event_handler(event_type handled_event, std::string  handle_code) :
			event_handler(handled_event, python), m_handle_code(std::move(handle_code)) {}

	const std::string& get_handle_code() const { return m_handle_code; }

	void handle(const event_handler::handle_arguments& args) final;

  private:
	const std::string m_handle_code;
};
