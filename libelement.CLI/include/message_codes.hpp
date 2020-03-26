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

		message_code(std::string name, std::string level) : name{ name }, level{ get_message_level(level) }
		{
		}

		std::string name;
		message_level level;

	private:
		message_level get_message_level(std::string level) {

			std::map<std::string, message_level>::iterator it = map_message_level.find(level);
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

			std::map<std::string, message_code>::iterator it = code_map.begin();
			for (auto& item : table) {
				auto& message = item.second;

				auto& name = toml::find<std::string>(message, "name");
				auto& level = toml::find<std::string>(message, "level");

				code_map.insert(it, std::pair<std::string, message_code>(item.first, message_code(name, level)));
			}
		}

		const message_code* get_code() const {

			return nullptr;
		}

	private:
		//message_code create_message_code(std::string data) {
		//	return message_code(data, cli::message_level::ERROR);
		//}

	private:
		std::map<std::string, message_code> code_map;
	};
}