#include "io.h"
#include <iostream>
#include <fstream>
#include <string>

// Read value from key
std::string read_string_from_ini(const std::string& key, const std::string& default_value)
{
	const std::string filename = "settings.ini";
	std::ifstream ini_ifstream(filename);
	if (!ini_ifstream.is_open())
		std::cerr << "read_string_from_ini failed to open file " << filename << "\n";
	else
	{
		std::string read_key;
		std::string read_value;
		while (ini_ifstream >> read_key)
		{
			if (read_key == key)
			{
				// Remove delimiter, read value, remove leading spaces. There might be a better solution.
				ini_ifstream >> read_value;
				std::getline(ini_ifstream, read_value);
				const size_t value_start{ read_value.find_first_not_of(' ') };
				read_value = read_value.substr(value_start, read_value.length() - value_start + 1);
				read_value = literal_to_newline(read_value);
				return read_value;
			}
		}
	}
	std::cerr << "read_string_from_ini unable to find value for key " << key << "\n";
	return default_value;
}

std::string literal_to_newline(std::string str)
{
	std::string temp{ str };
	size_t index{ 0 };
	size_t count_removed{ 0 };
	for (auto curr_char = str.begin(); curr_char != str.end() - 1; ++curr_char)
	{
		if (*curr_char == '\\' && *(curr_char + 1) == 'n')
		{
			temp.erase(index - count_removed, 2);
			temp.insert(index - count_removed, 1, '\n');
			count_removed++;
		}
		index++;
	}
	return temp;
}
