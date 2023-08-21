#ifndef UTILS_HPP
# define UTILS_HPP

# include "_defines.hpp"

//UTILS CLASS______________________________________________________________________________________________________
//Utils is a static class that provides general-purpose functions

class Utils
{
	public:
		static std::string	getLocalTime();
		static bool			isPrintableStr(const std::string& message);
		static std::string	trimBackline(const std::string &request);
		static bool         isEven(std::size_t index);
		static bool         isOdd(std::size_t index);
		static bool			isNum(const std::string limit);

		//TEMPLATES__________________________________________________________________________________________________
		template <typename T, typename P>
		static void         printMap(std::map<T, P> &map)
		{
			typename std::map<T, P>::iterator it;
			for (it = map.begin(); it != map.end(); ++it)
			{
				std::cout << it->first << " : " << it->second << std::endl;
			}
		}
};

#endif
