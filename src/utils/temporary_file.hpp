#pragma once

#include <fstream>
#include <filesystem>

/**
 * Simple implementation of temporary file
 */
class temporary_file : public std::fstream {
  public:
	temporary_file() : temporary_file(std::tmpnam(nullptr)) {}

	temporary_file(const std::string& file_name) : std::fstream(file_name,
																std::ios::in |
																std::ios::out |
																std::ios::trunc
	),
												   m_file_name() {}

	~temporary_file() { std::remove(m_file_name.c_str()); };

  private:
	const std::string m_file_name;
};
