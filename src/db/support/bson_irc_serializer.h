//
// Created by strejivo on 7/8/19.
//

#ifndef TWITCH_IRC_BSON_IRC_SERIALIZER_H
#define TWITCH_IRC_BSON_IRC_SERIALIZER_H

#include <bsoncxx/array/value.hpp>
#include <bsoncxx/document/view_or_value.hpp>
#include "../../irc/message.h"

class bson_irc_serializer {
  public:
	bson_irc_serializer() = delete;

	static bsoncxx::document::value serializeMessage(const irc::message& message);

	static irc::message deserializeMessage(const bsoncxx::document::view_or_value& bsonMessage);

  private:
	static bsoncxx::array::value serializeTags(const irc::tags_t& tags);

	static bsoncxx::array::value serializeParams(const std::vector<std::string>& params);

	static bsoncxx::document::value serializePrefix(const irc::prefix_t& prefix);

	static irc::tags_t deserializeTags(const bsoncxx::array::view& bsonTags);

	static std::vector<std::string> deserializeParams(const bsoncxx::array::view& bsonParams);

	static irc::prefix_t deserializePrefix(const bsoncxx::document::view_or_value& bsonPrefix);
};


#endif //TWITCH_IRC_BSON_IRC_SERIALIZER_H
