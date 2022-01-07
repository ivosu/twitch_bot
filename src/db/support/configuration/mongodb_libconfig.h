#pragma once

#include "../../mongodb_communicator.h"

// Forward declare libconfigs settings
namespace libconfig {
    class Setting;
}

class mongodb_libconfig : public mongodb_communicator::config {
public:
    mongodb_libconfig(const libconfig::Setting& setting);

    const std::string& host() const final {
        return m_host;
    }

    uint16_t port() const final {
        return m_port;
    }

    const std::string& username() const final {
        return m_username;
    }

    const std::string& auth() const final {
        return m_auth;
    }

    const std::string& db() const final {
        return m_db;
    }

private:
    std::string m_host;
    uint16_t m_port;
    std::string m_username;
    std::string m_auth;
    std::string m_db;
};
