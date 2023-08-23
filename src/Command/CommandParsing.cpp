#include "_defines.hpp"
#include "CommandParsing.hpp"

size_t string_expect_fn(std::string str, size_t i, int (*fn)(int))
{
    size_t begin = i;
    while (fn(str[i]) && i < str.size())
        i++;
    return i - begin;
}

void string_try_expect(std::string str, size_t &i, int (*fn)(int))
{
    try
    {
        size_t word_size = string_expect_fn(str, i, fn);
        i += word_size;
    }
    catch(const std::exception& e)
    {
        // std::cerr << e.what() << '\n';
    }
    
}

int isspace_irc(int c)
{
    return c == ' ';
}

int istrailing(int c)
{
    return (c != '\r' && c != '\n' && c != '\0');
}

int ismiddle(int c)
{
    return (c != ' ' && c != '\r' && c != '\n' && c != '\0');
}

size_t expect_space(std::string str, size_t &i)
{
    size_t spaces_size = string_expect_fn(str, i, isspace_irc);
    i += spaces_size;
    return spaces_size;
}

void expect_params(std::string str, size_t &i, std::vector<std::string> &params)
{
    if (str.at(i) == ':')
    {
        i++;
        size_t word_size = string_expect_fn(str, i, istrailing);
        std::string param(&str[i], &str[i + word_size]);
        i += word_size;
        params.push_back(param);
    }
    else
    {
        size_t word_size = string_expect_fn(str, i, ismiddle);
        if (word_size == 0)
            throw std::exception();
        std::string param(&str[i], &str[i + word_size]);
        i += word_size;
        params.push_back(param);
    }
}

std::string expect_command(std::string str, size_t &i)
{
    if (size_t word_size = string_expect_fn(str, i, std::isalpha))
    {
        std::string command(&str[i], &str[i + word_size]);
        i += word_size;
        return command;
    }
    else if (string_expect_fn(str, i, std::isdigit) >= 3)
    {
        std::string command_num(str[i], str[i + 2]);
        i += 3;
        return command_num;
    }
    throw std::exception();
}

std::vector<std::string> parse_message(std::string str)
{
    size_t i = 0;

    std::string command_name = expect_command(str, i);
    std::vector<std::string> params;
    params.push_back(command_name);

    while (i < str.size())
    {
        if (expect_space(str, i) == 0)
            break ;
        if (!(i < str.size()))
            break;
        expect_params(str, i, params);
    }
    
    // if (i < str.size())
    //     std::cout << "Didn't read all of the message" << std::endl;
    return params;
}