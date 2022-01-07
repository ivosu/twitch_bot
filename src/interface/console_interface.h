#pragma once

#include <irc/message.h>

class console_interface {
public:
    console_interface() = default;

    virtual ~console_interface() = default;

    virtual void show_message(const irc::message& message) = 0;
};
