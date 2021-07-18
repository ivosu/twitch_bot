#include "bson_handler_serializer.h"
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/exception/exception.hpp>

#define BSON_CHANNEL "channel"
#define BSON_HANDLER_TYPE "handler_type"
#define BSON_HANDLED_EVENT_TYPE "handled_event_type"
#define BSON_HANDLE_CODE "handle_code"

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;

static const std::map<event_handler::event_type, std::string> event_to_string = {
		{event_handler::event_type::message,      "message"},
		{event_handler::event_type::subscription, "subscription"},
		{event_handler::event_type::room_state,   "roomstate"},
};

static const std::map<std::string, event_handler::event_type> event_from_string = {
		{"message",      event_handler::event_type::message},
		{"subscription", event_handler::event_type::subscription},
		{"roomstate",    event_handler::event_type::room_state}
};

static const std::map<event_handler::handler_type, std::string> handler_type_to_string = {
		{event_handler::handler_type::cpp,    "cpp"},
		{event_handler::handler_type::python, "python"},
};

static const std::map<std::string, event_handler::handler_type> handler_type_from_string = {
		{"cpp",    event_handler::handler_type::cpp},
		{"python", event_handler::handler_type::python},
};

bsoncxx::document::value
bson_handler_serializer::serialize_python_handler(const python_event_handler& handler, const std::string& channel) {

	auto serialized_handler = document{};
	return serialized_handler << BSON_CHANNEL << channel
							  << BSON_HANDLER_TYPE << handler_type_to_string.at(handler.get_handler_type())
							  << BSON_HANDLED_EVENT_TYPE << event_to_string.at(handler.get_handled_event_type())
							  << BSON_HANDLE_CODE << handler.get_handle_code()
							  << finalize;
}

bsoncxx::document::value
bson_handler_serializer::serialize_handler(const event_handler& handler, const std::string& channel) {
	switch (handler.get_handler_type()) {
		case event_handler::python:
			return serialize_python_handler(dynamic_cast<const python_event_handler&>(handler), channel);
		case event_handler::cpp:
			throw unserializable_exception();
		default:
			// debug assert
			return bsoncxx::document::value(nullptr, 0, nullptr);
	}
}

std::pair<std::shared_ptr<python_event_handler>, std::string>
bson_handler_serializer::deserialize_python_handler(const bsoncxx::document::view& serialized_handler) {
	std::string channel;
	event_handler::event_type handled_event_type = event_handler::event_type::message;
	std::string handle_code;
	bool channel_set = false;
	bool handled_event_type_set = false;
	bool handle_code_set = false;
	#ifdef DEBUG
	bool handler_type_set = false;
	#endif //DEBUG
	try {
		for (const bsoncxx::document::element& el : serialized_handler) {
			if (el.key() == BSON_CHANNEL) {
				channel = el.get_utf8().value;
				channel_set = true;
			} else if (el.key() == BSON_HANDLED_EVENT_TYPE) {
				handled_event_type = event_from_string.at(std::string(el.get_utf8().value));
				handled_event_type_set = true;
			} else if (el.key() == BSON_HANDLER_TYPE) {
				#ifdef DEBUG
				// TODO assert že je handle type fakt python
				handler_type_set = true;
				#endif //DEBUG
			} else if (el.key() == BSON_HANDLE_CODE) {
				handle_code = el.get_utf8().value;
				handle_code_set = true;
			} else {
				throw deserialization_exception();
			}
		}
	} catch (const bsoncxx::exception& e) {
		throw deserialization_exception();
	}
	if (!channel_set || !handled_event_type_set || !handle_code_set) {
		throw deserialization_exception();
	}
	#ifdef DEBUG
	// TODO assert handler_type_set
	#endif //DEBUG
	return std::make_pair(
			std::make_shared<python_event_handler>(handled_event_type, handle_code),
			channel);
}

std::shared_ptr<python_event_handler>
bson_handler_serializer::deserialize_python_handler_only(const bsoncxx::document::view& serialized_handler) {
	event_handler::event_type handled_event_type = event_handler::event_type::message;
	std::string handle_code;
	bool handled_event_type_set = false;
	bool handle_code_set = false;
	#ifdef DEBUG
	bool handler_type_set = false;
	#endif //DEBUG
	try {
		for (const bsoncxx::document::element& el : serialized_handler) {
			if (el.key() == BSON_CHANNEL) {
				continue;
			} else if (el.key() == BSON_HANDLED_EVENT_TYPE) {
				handled_event_type = event_from_string.at(std::string(el.get_utf8().value));
				handled_event_type_set = true;
			} else if (el.key() == BSON_HANDLER_TYPE) {
				#ifdef DEBUG
				// TODO assert that handle type is really python
				handler_type_set = true;
				#endif //DEBUG
			} else if (el.key() == BSON_HANDLE_CODE) {
				handle_code = el.get_utf8().value;
				handle_code_set = true;
			} else {
				throw deserialization_exception();
			}
		}
	} catch (const bsoncxx::exception& e) {
		throw deserialization_exception();
	}
	if (!handled_event_type_set || !handle_code_set) {
		throw deserialization_exception();
	}
	#ifdef DEBUG
	// TODO assert handler_type_set
	#endif //DEBUG
	return std::make_shared<python_event_handler>(handled_event_type, handle_code);
}

std::pair<std::shared_ptr<event_handler>, std::string>
bson_handler_serializer::deserialize_handler(const bsoncxx::document::view& serialized_handler) {
	switch (handler_type_from_string.at(std::string(serialized_handler["handler_type"].get_utf8().value))) {
		case event_handler::handler_type::python:
			return deserialize_python_handler(serialized_handler);
		case event_handler::handler_type::cpp:
			throw unserializable_exception();
		default:
			// TODO assert
			return std::make_pair(std::shared_ptr<event_handler>(), "");
	}
}

std::shared_ptr<event_handler>
bson_handler_serializer::deserialize_handler_only(const bsoncxx::document::view& serialized_handler) {
	switch (handler_type_from_string.at(std::string(serialized_handler["handler_type"].get_utf8().value))) {
		case event_handler::handler_type::python:
			return deserialize_python_handler_only(serialized_handler);
		case event_handler::handler_type::cpp:
			throw unserializable_exception();
		default:
			// TODO assert
			return std::shared_ptr<event_handler>();
	}
}

bsoncxx::document::value bson_handler_serializer::get_query(const std::string& channel) {
	auto query = document{};
	return query << BSON_CHANNEL << channel << finalize;
}

bsoncxx::document::value
bson_handler_serializer::get_query(const std::string& channel, event_handler::event_type event_type) {
	auto query = document{};
	return query << BSON_CHANNEL << channel << BSON_HANDLED_EVENT_TYPE << event_to_string.at(event_type) << finalize;
}

const char* bson_handler_serializer::unserializable_exception::what() const noexcept {
	return "This type of handler is unserializable";
}

const char* bson_handler_serializer::deserialization_exception::what() const noexcept {
	return "Failed to deserialize handler";
}
