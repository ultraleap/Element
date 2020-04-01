#pragma once

#include <string>
#include <map>

#include <toml.hpp>

namespace libelement::cli
{
	enum class message_level : unsigned int
	{
		FATAL = 0,
		WARNING,
		ERROR,
		INFORMATION,
		VERBOSE,
		UNKNOWN = 0xFFFF
	};

	struct message_code
	{
		typedef const std::map<std::string, message_level>::const_iterator message_level_const_iterator;

		std::string code;
		std::string name;
		message_level level;
		
		message_code(std::string code, std::string name, const std::string& level)
			: code{ std::move(code) }, name{ std::move(name) }, level{ get_message_level(level) }
		{
		}

	private:
		static message_level get_message_level(const std::string& level)
		{
			static std::map<std::string, message_level> map_message_level =
			{
				{"Fatal", message_level::FATAL},
				{"Warning", message_level::WARNING},
				{"Error", message_level::ERROR},
				{"Information", message_level::INFORMATION},
				{"Verbose", message_level::VERBOSE}
			};
			
			message_level_const_iterator it = map_message_level.find(level);
			if (it != map_message_level.end())
				return it->second;

			return message_level::UNKNOWN;
		}
	};

	class message_codes 
	{
		typedef const std::map<std::string, message_code>::const_iterator message_code_const_iterator;
	public:
		message_codes(const std::string& path) 
		{			
			auto data = toml::parse(path);
			auto table = toml::get<toml::table>(data);

			message_code_const_iterator it = code_map.begin();
			for (const auto& item : table) 
			{
				auto name = toml::find<std::string>(item.second, "name");
				auto level = toml::find<std::string>(item.second, "level");
				code_map.insert(it, std::pair<std::string, message_code>(item.first, message_code(item.first, name, level)));
			}
		}

		const message_code* get_code(const std::string& code) const
		{
			message_code_const_iterator it = code_map.find(code);
			if (it != code_map.end())
				return &it->second;

			return nullptr;
		}

	private:
		std::map<std::string, message_code> code_map;
	};
}