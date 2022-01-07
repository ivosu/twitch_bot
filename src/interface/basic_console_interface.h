#pragma once

#include "console_interface.h"

class basic_console_interface : public console_interface {
public:
    void show_message(const irc::message& message) override;
};
