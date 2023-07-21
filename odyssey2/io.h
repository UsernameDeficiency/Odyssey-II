#pragma once
#include <string>
std::string read_string_from_ini(const std::string& key, const std::string& default_value = "0");

std::string literal_to_newline(std::string str);
