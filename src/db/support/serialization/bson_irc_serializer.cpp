#include "bson_irc_serializer.h"
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/exception/exception.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;

#define BSON_IRC_MESSAGE_COMMAND "command"
#define BSON_IRC_MESSAGE_PARAMS "params"
#define BSON_IRC_MESSAGE_PREFIX "prefix"
#define BSON_IRC_MESSAGE_TAGS "tags"

#define BSON_IRC_MESSAGE_PREFIX_MAIN "main"
#define BSON_IRC_MESSAGE_PREFIX_HOST "host"
#define BSON_IRC_MESSAGE_PREFIX_USER "user"

bsoncxx::array::value bson_irc_serializer::serialize_tags(const irc::tags_t& tags) {
    auto tags_bson_array = array{};

    for (const auto& tag : tags) {
        if (tag.second.has_value()) {
            tags_bson_array << open_document << tag.first << tag.second.value() << close_document;
        } else {
            tags_bson_array << tag.first;
        }
    }

    return tags_bson_array << finalize;
}

bsoncxx::array::value bson_irc_serializer::serialize_params(const std::vector<std::string>& params) {
    auto params_bson_array = array{};

    for (const std::string& param : params) {
        params_bson_array << param;
    }

    return params_bson_array << finalize;
}

bsoncxx::document::value bson_irc_serializer::serialize_prefix(const irc::prefix_t& prefix) {
    auto prefix_document = document{};

    prefix_document << BSON_IRC_MESSAGE_PREFIX_MAIN << prefix.main();
    if (prefix.user().has_value())
        prefix_document << BSON_IRC_MESSAGE_PREFIX_USER << prefix.user().value();
    if (prefix.host().has_value())
        prefix_document << BSON_IRC_MESSAGE_PREFIX_HOST << prefix.host().value();

    return prefix_document << finalize;
}

bsoncxx::document::value bson_irc_serializer::serialize_message(const irc::message& message) {
    auto message_bson = document{};

    message_bson << BSON_IRC_MESSAGE_COMMAND << message.command();
    if (!message.params().empty())
        message_bson << BSON_IRC_MESSAGE_PARAMS << serialize_params(message.params());
    if (!message.tags().empty())
        message_bson << BSON_IRC_MESSAGE_TAGS << serialize_tags(message.tags());
    if (message.prefix().has_value()) {
        message_bson << BSON_IRC_MESSAGE_PREFIX << serialize_prefix(message.prefix().value());
    }

    return message_bson << finalize;
}


irc::message bson_irc_serializer::deserialize_message(const bsoncxx::document::view_or_value& bson_message) {
    irc::tags_t tags;
    std::optional<irc::prefix_t> prefix;
    std::string command;
    std::vector<std::string> params;

    try {
        for (const bsoncxx::document::element& el : bson_message.view()) {
            if (el.key() == BSON_IRC_MESSAGE_COMMAND)
                command = el.get_utf8().value;
            else if (el.key() == BSON_IRC_MESSAGE_PARAMS)
                params = deserialize_params(el.get_array());
            else if (el.key() == BSON_IRC_MESSAGE_TAGS)
                tags = deserialize_tags(el.get_array());
            else if (el.key() == BSON_IRC_MESSAGE_PREFIX)
                prefix = deserialize_prefix(el.get_document().view());
            else {
                throw deserialization_exception();
            }
        }
    } catch (const bsoncxx::exception& e) {
        throw deserialization_exception();
    }

    if (command.empty()) {
        throw deserialization_exception();
    }

    return irc::message(tags, prefix, command, params);
}

irc::tags_t bson_irc_serializer::deserialize_tags(const bsoncxx::array::view& bson_tags) {
    irc::tags_t tags;

    try {
        for (const bsoncxx::array::element& el : bson_tags) {
            if (el.type() == bsoncxx::types::b_document::type_id) {
                bsoncxx::document::view doc = el.get_document().view();
                if (std::distance(doc.begin(), doc.end()) != 1) {
                    throw deserialization_exception();
                }
                tags.emplace(doc.begin()->key(), doc.begin()->get_utf8().value);
            } else {
                tags.emplace(el.get_utf8().value, std::nullopt);
            }
        }
    } catch (const bsoncxx::exception& e) {
        throw deserialization_exception();
    }

    return tags;
}

std::vector<std::string> bson_irc_serializer::deserialize_params(const bsoncxx::array::view& bson_params) {
    std::vector<std::string> params;

    try {
        for (const bsoncxx::array::element& el : bson_params) {
            params.emplace_back(el.get_utf8().value);
        }
    } catch (const bsoncxx::exception& e) {
        throw deserialization_exception();
    }

    return params;
}

irc::prefix_t bson_irc_serializer::deserialize_prefix(const bsoncxx::document::view_or_value& bson_prefix) {
    std::string main;
    std::optional<std::string> user, host;

    try {
        for (const bsoncxx::document::element& el : bson_prefix.view()) {
            if (el.key() == BSON_IRC_MESSAGE_PREFIX_MAIN) {
                main = el.get_utf8().value;
            } else if (el.key() == BSON_IRC_MESSAGE_PREFIX_USER) {
                user = el.get_utf8().value;
            } else if (el.key() == BSON_IRC_MESSAGE_PREFIX_HOST) {
                host = el.get_utf8().value;
            } else {
                throw deserialization_exception();
            }
        }
    } catch (const bsoncxx::exception& e) {
        throw deserialization_exception();
    }
    if (main.empty()) {
        throw deserialization_exception();
    }

    return irc::prefix_t(main, user, host);
}

bsoncxx::document::value bson_irc_serializer::serialize_query(const std::string& query) {
    try {
        return bsoncxx::from_json(query);
    } catch (const bsoncxx::exception& e) {
        throw query_serialization_exception();
    }
}

const char* bson_irc_serializer::deserialization_exception::what() const noexcept {
    return "Problem with deserialization";
}

const char* bson_irc_serializer::query_serialization_exception::what() const noexcept {
    return "Problem with converting query to bson";
}
