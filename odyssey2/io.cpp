#include<fstream>
#include<string>

// Create ifstream for reading file
void init_from_file()
{
	const std::string filename = "settings.ini";
	std::ifstream ini_ifstream(filename);
	if (!ini_ifstream.is_open())
	{
		// Failed to open file
	}
	else
	{
		// Use >> to read from file
	}
}