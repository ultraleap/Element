#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "compiler_message.hpp"

using namespace libelement::cli;

message_level compiler_message::get_level() const
{
    return level.has_value() ? level.value() : message_level::Information;
}

std::string compiler_message::serialize() const
{
    if (serialize_to_json) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartObject();

        writer.String(key_message_type);
        writer.String(key_message_value);

        if (type.has_value()) {
            writer.String(key_message_code);
            writer.Int(static_cast<int>(type.value()));
        }
        if (level.has_value()) {
            writer.String(key_message_level);
            writer.Int(static_cast<int>(level.value()));
        }
        writer.String(key_context);
        writer.String(context.c_str());
        writer.String(key_trace_stack);
        writer.StartArray();
        for (auto& stack_item : trace_stack) {
            writer.String(stack_item.get_message().c_str());
        }
        writer.EndArray();
        writer.EndObject();

        return buffer.GetString();
    }

    return context + "\n";
}
