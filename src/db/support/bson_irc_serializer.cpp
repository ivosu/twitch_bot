//
// Created by strejivo on 7/8/19.
//

#include "bson_irc_serializer.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/exception/exception.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;

bsoncxx::array::value bson_irc_serializer::serializeTags(const irc::tags_t& tags) {
	auto tagsBsonArray = array{};

	for (const auto& tag : tags) {
		if (tag.second.has_value()) {
			tagsBsonArray << open_document << tag.first << tag.second.value() << close_document;
		} else {
			tagsBsonArray << tag.first;
		}
	}

	return tagsBsonArray << finalize;
}

bsoncxx::array::value bson_irc_serializer::serializeParams(const std::vector<std::string>& params) {
	auto paramsBsonArray = array{};

	for (const std::string& param : params) {
		paramsBsonArray << param;
	}

	return paramsBsonArray << finalize;
}

bsoncxx::document::value bson_irc_serializer::serializePrefix(const irc::prefix_t& prefix) {
	auto prefixDocument = document{};

	prefixDocument << "main" << prefix.main();
	if (prefix.user().has_value())
		prefixDocument << "user" << prefix.user().value();
	if (prefix.host().has_value())
		prefixDocument << "host" << prefix.host().value();

	return prefixDocument << finalize;
}

bsoncxx::document::value bson_irc_serializer::serializeMessage(const irc::message& message) {
	auto messageBson = document{};

	messageBson << "command" << message.command();
	if (!message.params().empty())
		messageBson << "params" << serializeParams(message.params());
	if (!message.tags().empty())
		messageBson << "tags" << serializeTags(message.tags());
	if (message.prefix().has_value()) {
		messageBson << "prefix" << serializePrefix(message.prefix().value());
	}

	return messageBson << finalize;
}


irc::message bson_irc_serializer::deserializeMessage(const bsoncxx::document::view_or_value& bsonMessage) {
	irc::tags_t tags;
	std::optional<irc::prefix_t> prefix;
	std::string command;
	std::vector<std::string> params;

	try {
		for (const bsoncxx::document::element& el : bsonMessage.view()) {
			if (el.key() == "command")
				command = el.get_utf8().value;
			else if (el.key() == "params")
				params = deserializeParams(el.get_array());
			else if (el.key() == "tags")
				tags = deserializeTags(el.get_array());
			else if (el.key() == "prefix")
				prefix = deserializePrefix(el.get_document().view());
			else {
				// TODO throw
			}
		}
	} catch (const bsoncxx::exception& e) {
		// TODO throw
	}

	if (command.empty()) {
		// TODO throw
	}

	return irc::message(tags, prefix, command, params);
}

irc::tags_t bson_irc_serializer::deserializeTags(const bsoncxx::array::view& bsonTags) {
	irc::tags_t tags;

	try {
		for (const bsoncxx::array::element& el : bsonTags) {
			if (el.type() == bsoncxx::types::b_document::type_id) {
				bsoncxx::document::view doc = el.get_document().view();
				if (doc.length() != 1) {
					// TODO throw
				}
				tags.emplace(doc.begin()->key(), doc.begin()->get_utf8().value);
			} else {
				tags.emplace(el.get_utf8().value, std::nullopt);
			}
		}
	} catch (const bsoncxx::exception& e) {
		// TODO throw
	}

	return tags;
}

std::vector<std::string> bson_irc_serializer::deserializeParams(const bsoncxx::array::view& bsonParams) {
	std::vector<std::string> params;

	try {
		for (const bsoncxx::array::element& el : bsonParams) {
			params.emplace_back(el.get_utf8().value);
		}
	} catch (const bsoncxx::exception& e) {
		// TODO throw
	}

	return params;
}

irc::prefix_t bson_irc_serializer::deserializePrefix(const bsoncxx::document::view_or_value& bsonPrefix) {
	std::string main;
	std::optional<std::string> user, host;

	try {
		for (const bsoncxx::document::element& el : bsonPrefix.view()) {
			if (el.key() == "main") {
				main = el.get_utf8().value;
			} else if (el.key() == "user") {
				user = el.get_utf8().value;
			} else if (el.key() == "host") {
				host = el.get_utf8().value;
			} else {
				// TODO throw
			}
		}
	} catch (const bsoncxx::exception& e) {
		// TODO throw
	}
	if (main.empty()) {
		// TODO throw
	}

	return irc::prefix_t(main, user, host);
}
