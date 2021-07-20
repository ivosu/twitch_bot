#pragma once

#include "../../../handlers/event_handler.h"
#include "../../../handlers/python_event_handler.h"
#include <bsoncxx/document/value.hpp>

class bson_handler_serializer {
public:
    bson_handler_serializer() = delete;

    static bsoncxx::document::value serialize_handler(const event_handler& handler, const std::string& channel);

    static std::pair<std::shared_ptr<event_handler>, std::string>
    deserialize_handler(const bsoncxx::document::view& serialized_handler);

    static std::shared_ptr<event_handler>
    deserialize_handler_only(const bsoncxx::document::view& serialized_handler);

    static bsoncxx::document::value get_query(const std::string& channel);

    static bsoncxx::document::value get_query(const std::string& channel, event_handler::event_type event_type);

    class unserializable_exception : public std::exception {
    public:
        const char* what() const noexcept final;
    };

    class deserialization_exception : public std::exception {
    public:
        const char* what() const noexcept final;
    };

private:
    static bsoncxx::document::value
    serialize_python_handler(const python_event_handler& handler, const std::string& channel);

    static std::pair<std::shared_ptr<python_event_handler>, std::string>
    deserialize_python_handler(const bsoncxx::document::view& serialized_handler);

    static std::shared_ptr<python_event_handler>
    deserialize_python_handler_only(const bsoncxx::document::view& serialized_handler);
};
