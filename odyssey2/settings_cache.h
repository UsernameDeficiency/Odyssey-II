#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>

// Singleton cache for settings values read from settings file
class settings_cache
{
public:
	// Disallow copying/moving
	settings_cache(const settings_cache&) = delete;
	settings_cache& operator=(const settings_cache&) = delete;
	settings_cache(settings_cache&&) = delete;
	settings_cache& operator=(settings_cache&&) = delete;

	static settings_cache& instance()
	{
		static settings_cache instance;
		return instance;
	}

	template <typename T>
	T get(const std::string& key, const T& default_value) const
	{
		std::string read_value;

		if (const auto search = cache.find(key); search != cache.end())
			read_value = search->second;
		else
		{
			std::cerr << "settings_cache.get: unable to find value for key '" << key << "'\n";
			return default_value;
		}

		// Return value as type T
		std::istringstream iss(read_value);
		T converted_value{};
		if constexpr (std::is_same_v<T, std::string>)
		{
			return read_value;
		}
		else if (iss >> converted_value)
		{
			return converted_value;
		}
		else
		{
			std::cerr << "settings_cache.get: could not convert value '" << read_value << "' into type " << typeid(T).name() << " for key '" << key << "'\n";
			return default_value;
		}
	}

private:
	using cache_t = std::map<std::string, std::string>;

	settings_cache()
	{
		cache = cache_settings_file();
	}

	~settings_cache() = default;

	const std::string filename = "settings.ini";

	// Convert all occurrences of "\n" in str to newlines ('\n')
	static std::string literal_to_newline(const std::string& str)
	{
		std::string temp{ str };
		size_t index{ 0 };
		size_t count_removed{ 0 };
		for (auto curr_char = str.begin(); curr_char != str.end(); ++curr_char)
		{
			if (curr_char + 1 != str.end() && *curr_char == '\\' && *(curr_char + 1) == 'n')
			{
				temp.erase(index - count_removed, 2);
				temp.insert(index - count_removed, 1, '\n');
				count_removed++;
			}
			index++;
		}
		return temp;
	}

	cache_t cache;

	// Read all key-value pairs from settings file into cache
	cache_t cache_settings_file() const
	{
		std::ifstream ini_ifstream(filename);
		cache_t settings_map{};

		if (!ini_ifstream.is_open())
		{
			std::cerr << "cache_settings_file failed to open file " << filename << "\n";
			return settings_map;
		}

		std::string read_key;
		std::string read_value;
		while (ini_ifstream >> read_key)
		{
			// Discard commented lines and sections
			if (!read_key.empty() && (read_key[0] == ';' || read_key[0] == '['))
			{
				std::getline(ini_ifstream, read_key);
				continue;
			}

			// Remove delimiter
			char delimiter;
			ini_ifstream >> delimiter;
			if (delimiter != '=')
			{
				std::cerr << "cache_settings_file expected '=' delimiter after key " << read_key << ", but got '" << delimiter << "'\n";
				continue;
			}

			// Read value, remove leading spaces
			std::getline(ini_ifstream, read_value);
			const size_t value_start{ read_value.find_first_not_of(' ') };
			if (value_start == std::string::npos)
				continue; // No value found after delimiter
			read_value = read_value.substr(value_start, read_value.length() - value_start);
			read_value = literal_to_newline(read_value);

			settings_map[read_key] = read_value;
		}

		return settings_map;
	}
};

// Get setting value for given key from settings cache.
// If key is not found or conversion to type T fails, return default_value.
template <typename T>
T get_setting(const std::string& key, const T& default_value)
{
	return settings_cache::instance().get<T>(key, default_value);
}
