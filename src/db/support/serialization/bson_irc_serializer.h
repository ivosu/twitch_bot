#ifndef TWITCH_IRC_BSON_IRC_SERIALIZER_H
#define TWITCH_IRC_BSON_IRC_SERIALIZER_H

#include <bsoncxx/array/value.hpp>
#include <bsoncxx/document/view_or_value.hpp>
#include "../../../irc/message.h"

class bson_irc_serializer {
  public:
	bson_irc_serializer() = delete;

	static bsoncxx::document::value serialize_message(const irc::message& message);

	static irc::message deserialize_message(const bsoncxx::document::view_or_value& bson_message);

	static bsoncxx::document::value serialize_query(const std::string& query);

	class deserialization_exception : public std::exception {
	  public:
		const char* what() const noexcept final;
	};

	class query_serialization_exception : public std::exception {
		const char* what() const noexcept final;
	};

  private:
	static bsoncxx::array::value serialize_tags(const irc::tags_t& tags);

	static bsoncxx::array::value serialize_params(const std::vector<std::string>& params);

	static bsoncxx::document::value serialize_prefix(const irc::prefix_t& prefix);

	static irc::tags_t deserialize_tags(const bsoncxx::array::view& bson_tags);

	static std::vector<std::string> deserialize_params(const bsoncxx::array::view& bson_params);

	static irc::prefix_t deserialize_prefix(const bsoncxx::document::view_or_value& bson_prefix);
};


#endif //TWITCH_IRC_BSON_IRC_SERIALIZER_H
