#include "mongodb_libconfig.h"

const std::string& mongodb_libconfig::host() const {
	return m_host;
}

uint16_t mongodb_libconfig::port() const {
	return m_port;
}

const std::string& mongodb_libconfig::username() const {
	return m_username;
}

const std::string& mongodb_libconfig::auth() const {
	return m_auth;
}

const std::string& mongodb_libconfig::db() const {
	return m_db;
}

mongodb_libconfig::mongodb_libconfig(const libconfig::Setting& setting) {
	setting.lookupValue("host", m_host);
	setting.lookupValue("port", (unsigned int&) m_port);
	setting.lookupValue("username", m_username);
	setting.lookupValue("auth", m_auth);
	setting.lookupValue("db", m_db);
}
