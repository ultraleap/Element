#include <iostream>

#include <CLI/CLI.hpp>
#include <compiler_message.hpp>

int main()
{
	cli::compiler_message message(10, cli::message_level::ERROR, "Hello World!", std::vector<cli::trace_site>{});

	std::cout << message.serialize();
    //std::cout << R"({"MessageCode":null,"MessageLevel":null,"Context":"Hello World!","TraceStack":[]})";
}