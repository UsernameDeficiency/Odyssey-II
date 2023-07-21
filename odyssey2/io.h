#pragma once
#include <string>

// Read value from key from INI-inspired settings file
std::string read_string_from_ini(const std::string& key, const std::string& default_value = "0");

// Convert all occurences of "\n" in str to newlines ('\n')
static std::string literal_to_newline(std::string str);
