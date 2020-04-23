#pragma once

#include <string>
#include <map>

#include <toml.hpp>

namespace libelement::cli
{
	enum class message_level : unsigned int
	{
		VERBOSE = 0,
		INFORMATION,
		WARNING,
		ERROR,
		FATAL,
		UNKNOWN = 0xFFFF
	};

	enum class message : unsigned int
	{
		SUCCESS = 0,
		SERIALIZATION_ERROR = 1,
		MULTIPLE_DEFINITIONS = 2,
		INVALID_COMPILE_TARGET = 3,
		INTRINSIC_NOT_IMPLEMENTED = 4,
		LOCAL_SHADOWING = 5,
		ARGUMENT_COUNT_MISMATCH = 6,
		IDENTIFIER_NOT_FOUND = 7,
		CONSTRAINT_NOT_SATISFIED = 8,
		PARSE_ERROR = 9,
		INVALID_BOUNDARY_FUNCTION_INTERFACE = 10,
		CIRCULAR_COMPILATION = 11,
		BOUNDARY_MAP_MISSING = 12,
		MISSING_PORTS = 13,
		TYPE_ERROR = 14,
		INVALID_IDENTIFIER = 15,
		INVALID_EXPRESSION = 16,
		INVALID_RETURN_TYPE = 17,
		REDUNDANT_QUALIFIER = 18,
		STRUCT_CANNOT_HAVE_RETURN_TYPE = 19,
		INTRINSIC_CANNOT_HAVE_BODY = 20,
		MISSING_FUNCTION_BODY = 21,
		CANNOT_BE_USED_AS_INSTANCE_FUNCTION = 22,
		UNKNOWN_ERROR = 9999
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