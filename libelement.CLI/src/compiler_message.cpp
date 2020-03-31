#include "compiler_message.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace libelement::cli;

const std::string compiler_message::serialize() {

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	writer.StartObject();
	writer.String(compiler_message::key_message_code);
	writer.Int(message_code);
	writer.String(compiler_message::key_message_level);
	writer.Int(static_cast<int>(message_level));
	writer.String(compiler_message::key_context);
	writer.String(context.c_str());
	writer.String(compiler_message::key_trace_stack);
	writer.StartArray();
	for (auto& stack_item : trace_stack) {
		writer.String(stack_item.message().c_str());
	}
	writer.EndArray();
	writer.EndObject();

	return buffer.GetString();
}