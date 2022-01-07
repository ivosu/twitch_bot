#include "mongodb_libconfig.h"

#include <libconfig.h++>

constexpr const char* host_string = "host";
constexpr const char* port_string = "port";
constexpr const char* username_string = "username";
constexpr const char* auth_string = "auth";
constexpr const char* db_string = "db";

mongodb_libconfig::mongodb_libconfig(const libconfig::Setting& setting)
    // Default init
    : m_host{}, m_port{0}, m_username{}, m_auth{}, m_db{} {
    setting.lookupValue(host_string, m_host);
    setting.lookupValue(port_string, (unsigned int&) m_port);
    setting.lookupValue(username_string, m_username);
    setting.lookupValue(auth_string, m_auth);
    setting.lookupValue(db_string, m_db);
}
