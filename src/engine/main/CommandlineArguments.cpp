#include "CommandlineArguments.h"
#include "engine/console.h"
#include <cstring>

CommandlineArguments::CommandlineArguments(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i)
    {
        if (argv[i][0] == '-' && i + 1 < argc)
        {
            if (strlen(argv[i]) > 2)
            {
                m_Arguments.insert(
                    std::make_pair(std::string(argv[i] + 1, 1), argv[i + 1])
                );
            }
            else
            {
                m_Arguments.insert(std::make_pair(argv[i] + 1, argv[i + 1]));
                ++i;
            }
        }
        else if (auto valPtr = strstr(argv[i], "="); valPtr)
        {
            m_Arguments.insert(
                std::make_pair(std::string(argv[i], argv[i] - valPtr), valPtr + 1)
            );
        }
        else
        {
            m_Arguments.insert(std::make_pair("__any", argv[i]));
        }
    }
}

std::vector<std::string> CommandlineArguments::GetAll(Argument arg) const
{
    static std::map<Argument, std::string> ArgumentToString = {
        {Argument::HomeDirectory, "u"},
        {Argument::LogFilename, "g"},

        {Argument::Any, "__any"},
    };

    if (ArgumentToString.find(arg) == ArgumentToString.end())
    {
        conoutf("CommandlineArguments::GetAll: No such argument");
    }

    return GetAll(ArgumentToString.at(arg));
}

std::vector<std::string> CommandlineArguments::GetAll(const std::string& arg) const
{
    std::vector<std::string> arglist;
    for (auto& argPair : m_Arguments)
    {
        if (argPair.first == arg)
        {
            arglist.push_back(argPair.second);
        }
    }

    return arglist;
}

std::string CommandlineArguments::Get(Argument arg) const
{
    auto args = GetAll(arg);
    if (args.size())
    {
        return args[0];
    }

    return "";
}
