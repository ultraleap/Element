#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "compiler_message.hpp"

using namespace libelement::cli;

std::string compiler_message::serialize() const
{

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	writer.StartObject();
	writer.String(key_message_code);
	writer.Int(static_cast<int>(message_code));
	writer.String(key_message_level);
	writer.Int(static_cast<int>(level));
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
