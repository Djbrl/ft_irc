#include "Utils.hpp"

std::string Utils::getLocalTime()
{
	time_t		rawTime = std::time(nullptr);
	struct tm	*timeInfo;
	char		buffer[80];
	
	timeInfo = std::localtime(&rawTime);
	std::time(&rawTime);
	std::strftime(buffer, sizeof(buffer), "%c", timeInfo);
	std::string time = buffer;
	std::string hours = time.substr(11, 2);
	std::string minutes = time.substr(14, 2);
	std::string localTime = BBLUE + hours + ":" + minutes + RESET + " -!- ";
	return localTime;
}

bool		Utils::isPrintableStr(const std::string& message)
{
	for (size_t i = 0; i < message.length(); ++i)
	{
		char c = message[i];
		if (c < 32 || c > 126)
			return false;
	}
	return true;
}

std::string	Utils::trimBackline(const std::string &request)
{
	return request.substr(0, request.length() - 1);
}