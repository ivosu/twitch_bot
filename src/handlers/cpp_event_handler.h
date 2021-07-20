#pragma once

#include <functional>
#include "event_handler.h"

class cpp_event_handler : public event_handler {
    using handle_function = std::function<void(const handle_arguments&)>;

public:
    cpp_event_handler(event_type handled_event, handle_function handler) :
            event_handler(handled_event, cpp), m_handler(std::move(handler)) {}

    void handle(const handle_arguments& args) final { m_handler(args); }

private:
    handle_function m_handler;
};
