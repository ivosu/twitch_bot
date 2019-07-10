//
// Created by strejivo on 7/3/19.
//

#include "mongodb_communicator.h"
#include "support/bson_irc_serializer.h"
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <bsoncxx/types.hpp>
#include <iostream>
#include <bsoncxx/json.hpp>


bool mongodb_communicator::save_message(const irc::message& message) {
	bsoncxx::document::value message_bson = bson_irc_serializer::serialize_message(message);
	irc::message deserialized_message = bson_irc_serializer::deserialize_message(message_bson.view());
	if (deserialized_message != message) {
		std::cerr << "Deserialized message differs from original message" << std::endl << message.to_irc_message()
				  << std::endl << bsoncxx::to_json(message_bson) << std::endl << deserialized_message.to_irc_message()
				  << std::endl << std::endl;
	}
	try {
		auto result = m_messages_collection.insert_one(message_bson.view());
	} catch (const mongocxx::bulk_write_exception& e) {
		return false;
	}
	return true;
}

std::vector<irc::message> mongodb_communicator::load_messages(const std::string& query) {
	try {
		return load_messages(bson_irc_serializer::serialize_query(query));
	} catch (const bson_irc_serializer::query_serialization_exception& e) {
		throw invalid_query();
	}
}

mongodb_communicator::mongodb_communicator(const std::string& host, uint16_t port, const std::string& username,
										   const std::string& auth) {
	std::string userinfo = username;
	if (!auth.empty()) {
		userinfo += ":" + auth;
	}
	std::string uri_str = "mongodb://";
	if (!userinfo.empty()) {
		uri_str += userinfo + "@";
	}
	uri_str += host + ":" + std::to_string(port);
	mongocxx::uri uri{ uri_str };

	m_client = mongocxx::client(uri);
	m_messages_collection = m_client["twitch_bot"]["messages"];
}

std::vector<irc::message> mongodb_communicator::load_messages(const bsoncxx::document::view_or_value& query) {
	std::vector<irc::message> result_messages;

	try {
		mongocxx::cursor cursor = m_messages_collection.find(query);
		try {
			for (const bsoncxx::document::view& doc : cursor) {
				try {
					result_messages.push_back(bson_irc_serializer::deserialize_message(doc));
				} catch (const bson_irc_serializer::deserialization_exception& e) {
					// TODO log
				}
			}
		} catch (const mongocxx::query_exception& e) {
			return std::vector<irc::message>();
		}
	} catch (const mongocxx::logic_error& e) {
		throw invalid_query();
	}

	return result_messages;
}
