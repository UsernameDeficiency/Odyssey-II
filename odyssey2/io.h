#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

// Convert all occurences of "\n" in str to newlines ('\n')
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

// Read value from key from INI-inspired settings file
template <typename T>
T read_value_from_ini(const std::string& key, const T& default_value)
{
	const std::string filename = "settings.ini";
	std::ifstream ini_ifstream(filename);
	if (!ini_ifstream.is_open())
	{
		std::cerr << "read_value_from_ini failed to open file " << filename << "\n";
		return default_value;
	}

	std::string read_key;
	std::string read_value;
	while (ini_ifstream >> read_key)
	{
		if (read_key == ";")
			std::getline(ini_ifstream, read_key); // Discard commented line
		else if (read_key == key)
		{
			// Remove delimiter
			char delimiter;
			ini_ifstream >> delimiter;
			if (delimiter != '=')
			{
				std::cerr << "read_value_from_ini expected '=' delimiter after key " << key << ", but got '" << delimiter << "'\n";
				return default_value;
			}

			// Read value, remove leading spaces.
			std::getline(ini_ifstream, read_value);
			const size_t value_start{ read_value.find_first_not_of(' ') };
			if (value_start == std::string::npos)
				return default_value; // No value found after delimiter
			read_value = read_value.substr(value_start, read_value.length() - value_start);
			read_value = literal_to_newline(read_value);

			// Return found value as type T
			std::istringstream iss(read_value);
			T converted_value;
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
				std::cerr << "read_value_from_ini: conversion failed for key " << key << "\n";
				return default_value;
			}
		}
	}

	std::cerr << "read_value_from_ini unable to find value for key " << key << "\n";
	return default_value;
}
