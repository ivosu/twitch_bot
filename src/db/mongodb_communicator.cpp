#include "mongodb_communicator.h"
#include "support/serialization/bson_irc_serializer.h"
#include "support/serialization/bson_handler_serializer.h"
#include "../utils/logging.hpp"
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include <mongocxx/exception/query_exception.hpp>
#include <bsoncxx/types.hpp>
#include <iostream>
#include <bsoncxx/json.hpp>
#include <assert.h>


bool mongodb_communicator::save_message(const irc::message& message) {
	bsoncxx::document::value message_bson = bson_irc_serializer::serialize_message(message);
	try {
		irc::message deserialized_message = bson_irc_serializer::deserialize_message(message_bson.view());
		if (deserialized_message != message) {
			 LOG_ERROR("Deserialized message differs from original message");
			 LOG_ERROR(message.to_irc_message());
			 LOG_ERROR(bsoncxx::to_json(message_bson));
			 LOG_ERROR(deserialized_message.to_irc_message());
		 }
	} catch (const bson_irc_serializer::deserialization_exception& e) {
		std::cout << bsoncxx::to_json(message_bson) << std::endl;
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

mongodb_communicator::mongodb_communicator(const config& config) {
	std::string userinfo = config.username();
	if (!config.auth().empty()) {
		userinfo += ":" + config.auth();
	}
	std::string uri_str = "mongodb://";
	if (!userinfo.empty()) {
		uri_str += userinfo + "@";
	}
	uri_str += config.host() + ":" + std::to_string(config.port());
	mongocxx::uri uri{uri_str};

	m_client = mongocxx::client(uri);
	m_messages_collection = m_client[config.db()]["messages"];
	m_handlers_collection = m_client[config.db()]["handlers"];
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

bool mongodb_communicator::save_or_update_handler(const event_handler& handler, const std::string& channel) {
	bsoncxx::document::view_or_value serialized_handler;
	try{
		serialized_handler = bson_handler_serializer::serialize_handler(handler, channel);
	}
	catch (const bson_handler_serializer::unserializable_exception& e) {
		return false;
	}

	bsoncxx::document::view_or_value query = bson_handler_serializer::get_query(channel, handler.get_handled_event_type());
	auto maybe_result = m_handlers_collection.find_one(query);
	if (maybe_result) {
		try {
			m_handlers_collection.update_one(query, serialized_handler);
		} catch (const mongocxx::bulk_write_exception& e) {
			// todo log
			return false;
		} catch (const mongocxx::logic_error& e){
			// todo log
			return false;
		}
	}
	else {
		try {
			auto result = m_handlers_collection.insert_one(serialized_handler);
			return true;
		} catch (const mongocxx::bulk_write_exception& e) {
			return false;
		}
	}
	return false;
}

std::vector<std::shared_ptr<event_handler>> mongodb_communicator::load_handlers(const std::string& channel) {
	bsoncxx::document::view query = bson_handler_serializer::get_query(channel);
	std::vector<std::shared_ptr<event_handler>> matchedHandlers;
	mongocxx::cursor cursor = m_handlers_collection.find(query);
	for (auto doc : cursor) {
		matchedHandlers.push_back(bson_handler_serializer::deserialize_handler_only(doc));
	}
	return matchedHandlers;
}

std::shared_ptr<event_handler>
mongodb_communicator::load_handler(const std::string& channel, event_handler::event_type event_type) {
	bsoncxx::document::view query = bson_handler_serializer::get_query(channel, event_type);
	std::shared_ptr<event_handler> matched_handler;
	#ifdef DEBUG
	mongocxx::cursor cursor = m_handlers_collection.find(query);
	// TODO redo assert
	assert(std::distance(cursor.begin(), cursor.end()) == 1);
	return bson_handler_serializer::deserialize_handler_only(*cursor.begin());
	#else
	return bson_handler_serializer::deserialize_handler_only(m_handlers_collection.find_one(query));
	#endif // DEBUG
}
