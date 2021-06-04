//STD
#include <iostream>

//LIBS
#include <toml.hpp>
#include <fmt/format.h>
#include <element/common.h>
#include <CLI/CLI.hpp>
#ifdef WIN32
#include "Windows.h"
#endif
//SELF
#include "command.hpp"
#include "compiler_message.hpp"

using namespace libelement::cli;

void log_callback(const element_log_message* const msg, void* user_data)
{
    auto message_code = msg->message_code;

    std::string formatted_error = fmt::format("ELE {} ------------- {}\n{}\n\n",
                                              msg->message_code,
                                              msg->filename ? msg->filename : "<no filename>",
                                              msg->message ? msg->message : "<no message>");

    if (msg->line > 0)
    {
        formatted_error += fmt::format("{}|{}\n", msg->line, msg->line_in_source ? msg->line_in_source : "<no source line>");

        if (msg->character > 0)
        {
            const auto digits = std::to_string(msg->line).size();
            formatted_error += fmt::format("{}{}",
                                           std::string(digits + msg->character, ' '),
                                           std::string(msg->length, '^'));
        }
    }

    // todo: hack to force parse errors
    auto log = compiler_message(
        message_code, formatted_error,
        static_cast<command*>(user_data)->get_common_arguments().log_json);
    std::cout << log.serialize() << std::endl;

    // todo: might want to do this differently in the future
    if (msg->related_log_message)
        log_callback(msg->related_log_message, user_data);
}

void command_callback(command& command)
{
    #if !defined(NDEBUG) && defined(WIN32)
          //TODO: JM - TEMPORARY - Remove me later
          SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
  SEM_NOOPENFILEERRORBOX);

          _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
          _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

          _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
          _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

          _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
          _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  #endif

    // set a log callback, so that when we get errors, messages are logged
    command.set_log_callback(log_callback, static_cast<void*>(&command));

    // feedback request
    const auto request = compiler_message(
        command.as_string(), command.get_common_arguments().log_json);
    std::cout << request.serialize() << std::endl;

    // callback in case we need access to the command for some compiler_message
    // generation shenanigans
    const auto input = compilation_input(command.get_common_arguments());
    const auto response = command.execute(input);
    std::cout << response.serialize() << std::endl;
    command.set_log_callback(nullptr, nullptr);
}

int main(const int argc, char** argv)
{
    #if !defined(NDEBUG)
    std::string args;
    for (int i = 0; i < argc; ++i)
    {
        args += argv[i];
        args += " ";
    }
    std::cout << "Raw CLI Arguments: " << args << std::endl << std::endl;
    #endif
    
    // parse arguments and construct exactly one required command
    CLI::App app{ "CLI interface for libelement" };
    app.set_help_all_flag("--help-all", "Expand all help");
    app.require_subcommand(1, 1);

    command::configure(app, command_callback);

    CLI11_PARSE(app, argc, argv);
}