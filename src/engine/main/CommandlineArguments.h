#pragma once
#include <map>
#include <vector>
#include <string>

enum class Argument
{
    HomeDirectory,
    LogFilename,

    Any
};

class CommandlineArguments
{
public:
    CommandlineArguments(int argc, char** argv);

    std::string Get(Argument arg) const;
    std::vector<std::string> GetAll(Argument arg) const;

private:
    std::vector<std::string> GetAll(const std::string& arg) const;
    std::multimap<std::string, std::string> m_Arguments;
};


