#pragma once

#include <vector>
#include <string>
#include<map>

#include <toml.hpp>

namespace cli {

	enum class message_level : unsigned int {
		FATAL = 0,
		WARNING,
		ERROR,
		INFORMATION,
		VERBOSE,
		UNKNOWN = 0xFFFF
	};

	struct message_code {

		message_code(const std::string& name, const std::string& level) 
			: name{ name }, level{ get_message_level(level) }
		{
		}

		std::string name;
		message_level level;

	private:
		message_level get_message_level(const std::string& level) const {

			std::map<std::string, message_level>::const_iterator it = map_message_level.find(level);
			if (it != map_message_level.end())
				return it->second;

			return cli::message_level::UNKNOWN;
		}

		static std::map<std::string, message_level> map_message_level;
	};

	class message_codes {

	public:
		message_codes(std::string path) {

 			const auto data = toml::parse(path);
			const auto table = toml::get<toml::table>(data);

			std::map<std::string, message_code>::const_iterator it = code_map.begin();
			for (auto& item : table) {
				auto& message = item.second;

				auto& name = toml::find<std::string>(message, "name");
				auto& level = toml::find<std::string>(message, "level");

				code_map.insert(it, std::pair<std::string, message_code>(item.first, message_code(name, level)));
			}
		}

		const message_code* const get_code(std::string code) const {

			std::map<std::string, message_code>::const_iterator it = code_map.find(code);
			if (it != code_map.end())
				return &(it->second);

			return nullptr;
		}

	private:
		std::map<std::string, message_code> code_map;
	};
}