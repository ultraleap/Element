#pragma once

#include <command.hpp>
#include <compiler_message.hpp>

namespace libelement::cli 
{
	//not sure if this is required, depends on how we choose to pipe information into libelement
	class compilation_input
	{
		common_command_arguments common_arguments;
		std::function<void(const compiler_message&)> callback;

	public:
		compilation_input(common_command_arguments common_arguments, std::function<void(const compiler_message&)> callback)
			: common_arguments{ std::move(common_arguments) }, callback{ std::move(callback) }
		{
		}
	};
}