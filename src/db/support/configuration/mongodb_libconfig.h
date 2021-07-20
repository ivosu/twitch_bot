#pragma once

#include "../../mongodb_communicator.h"
#include <libconfig.h++>

class mongodb_libconfig : public mongodb_communicator::config {
  public:
	mongodb_libconfig(const libconfig::Setting& setting);

	const std::string& host() const final;

	uint16_t port() const final;

	const std::string& username() const final;

	const std::string& auth() const final;

	const std::string& db() const final;

  private:
	std::string m_host;
	uint16_t m_port;
	std::string m_username;
	std::string m_auth;
	std::string m_db;
};
