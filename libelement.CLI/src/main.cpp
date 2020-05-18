#include <iostream>

#include <CLI/CLI.hpp>
#include <toml.hpp>
#include <fmt/format.h>
#include <element/common.h>

#include "compiler_message.hpp"
#include "command.hpp"

using namespace libelement::cli;

void log_callback(const element_log_message* const message)
{
	auto message_code = message->message_code;

	std::string msg_type;
	switch (message->stage)
	{
	    case ELEMENT_STAGE_INVALID: msg_type = "libelement in invalid state\n"; break;
	    case ELEMENT_STAGE_MISC: msg_type = "Misc Message\n"; break;
		case ELEMENT_STAGE_TOKENISER: msg_type = "Tokeniser Message\n"; break;
	    case ELEMENT_STAGE_PARSER: msg_type = "Parser Message\n"; break;
		case ELEMENT_STAGE_COMPILER: msg_type = "Compiler Message\n"; break;
		case ELEMENT_STAGE_EVALUATOR: msg_type = "Evaluator Message\n"; break;
		default: msg_type = "Unknown Message\n"; break;
	}

	std::string msg_info;

	if (message->stage != ELEMENT_STAGE_MISC) {
		if (message_code == ELEMENT_OK) {
			msg_info = msg_type + fmt::format("libelement result: {}\nfile: {}\n",
				message->message_code, message->filename ? message->filename : "");
		}
		else
		{
			msg_info = msg_type + fmt::format("libelement result: {}\nfile: {}:{},{} length {}\n",
				message->message_code, message->filename ? message->filename : "", message->line, message->character, message->length);
		}
	}

	std::string message_with_info = msg_info + message->message;

	//todo: hack to force parse errors
	auto log = compiler_message(static_cast<message_type>(message_code), message_with_info);
	std::cout << log.serialize() << std::endl;

	//todo: might want to do this differently in the future
	if (message->related_log_message)
	    log_callback(message->related_log_message);
}

void command_callback(const command& command) 
{
#ifndef NDEBUG
	//TODO: JM - TEMPORARY - Remove me later
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
	
	//set a log callback, so that when we get errors, messages are logged
	command.set_log_callback(log_callback);

	//feedback request
	const auto request = compiler_message(command.as_string());
	std::cout << request.serialize() << std::endl;

	//callback in case we need access to the command for some compiler_message generation shenanigans
	const auto input = compilation_input(command.get_common_arguments());
	const auto response = command.execute(input);
	std::cout << response.serialize() << std::endl;
} 

int main(const int argc, char** argv)
{
	//parse arguments and construct exactly one required command
	CLI::App app{ "CLI interface for libelement" };
	app.set_help_all_flag("--help-all", "Expand all help");
	app.require_subcommand(1, 1);

	command::configure(app, command_callback);

	CLI11_PARSE(app, argc, argv);
}