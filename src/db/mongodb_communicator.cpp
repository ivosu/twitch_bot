//
// Created by strejivo on 7/3/19.
//

#include "mongodb_communicator.h"
#include "support/bson_irc_serializer.h"
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>




bool mongodb_communicator::saveMessage(const irc::message& message) {
	bsoncxx::document::value messageBson = bson_irc_serializer::serializeMessage(message);
	std::cout << bsoncxx::to_json(messageBson) << std::endl;

	mongocxx::database dat = m_client["twitch_bot"];
	mongocxx::collection coll = dat["messages"];
	try {
		auto result = coll.insert_one(messageBson.view());
	} catch (const mongocxx::bulk_write_exception& e) {
		return false;
	}
	return true;
}

std::vector<irc::message> mongodb_communicator::loadMessages(const std::string& querry) {
	return std::vector<irc::message>();
}

mongodb_communicator::mongodb_communicator(const std::string& host, uint16_t port, const std::string& username,
										   const std::string& auth) : db_communicator(host, port, username, auth) {
	mongocxx::uri uri{};
	//mongocxx::uri uri{"mongodb://" + host + ":" + std::to_string(port)}; // TODO add username and auth
	m_client = mongocxx::client(uri);
}