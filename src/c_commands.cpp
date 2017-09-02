//
// Copyright(C) 2017 Alex Mayfield
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Console command.
//

#include <unordered_map>

#include "c_commands.h"

#include "c_console.h"
#include "doomtype.h"
#include "i_system.h"
#include "m_config.h"

namespace theta
{

namespace console
{

static void CmdError(CommandArguments args)
{
    if (args.size() > 1)
    {
        I_Error("Console used error command: %s\n", args.at(1));
    }
    else
    {
        I_Error("Console used error command\n");
    }
}

static void CmdGet(CommandArguments args)
{
    if (args.size() < 2)
    {
        console::printf("get <variable>\n");
        return;
    }

    std::string var = args.at(1);
    std::string data;
    if (M_NiceGetVariableAsString(args.at(1), data))
    {
        M_NiceGetVariableAsString(var, data);
        console::printf("%s = %s\n", var.c_str(), data.c_str());
    }
    else
    {
        console::printf("unknown variable: %s\n", var.c_str());
    }
}

// Command to get a specific configuration variable
static void CmdSet(CommandArguments args)
{
    if (args.size() < 3)
    {
        console::printf("set <variable> <value>\n");
    }

    const char* var = args.at(1).c_str();
    const char* value = args.at(2).c_str();
    if (!M_SetVariable(var, value))
    {
        console::printf("error: unknown variable %s\n", var);
    }
}

// Get the global commands instance.
Commands& Commands::Instance()
{
    static Commands singleton;
    return singleton;
}

// Initialize console commands.
Commands::Commands() : command_map({
    { "error", CmdError },
    { "get", CmdGet },
    { "set", CmdSet }
}) { }

// Add a new command to the console.
void Commands::Add(const char* name, CommandImplementation func)
{
    this->command_map[name] = func;
}

// Extract and execuate a command line.
void Commands::Execute(const std::string& line)
{
    // This method handles argument parsing for you, so just hand it a
    // command line and it will do the right thing.
    //
    // There is basic quoted string and backslash escaping
    // functionality built in.  Incomplete quoted strings and escapes
    // with no character following it are considered errors.
    //
    // Please think very carefully about extending the functionality of
    // the parser.  This isn't meant to even approach the flexibility
    // of your average shell script parser.  If you need something
    // non-trivial, please use the dedicated scripting language.

    CommandArguments args;
    std::string arg;

    // States for the state machine.
    enum class state {
        Normal,
        QuotedString,
        EscapedChar,
        EscapedQuotedString,
    } state = state::Normal;

    // Pump the state machine until we get an arguments list.
    for (auto it = line.begin();it != line.end();++it)
    {
        auto& ch = *it;

        switch (state)
        {
        case state::Normal:
            if (ch == '"')
            {
                if (!arg.empty())
                {
                    // Split off an argument before the string begins.
                    args.push_back(arg);
                    arg.clear();
                }
                state = state::QuotedString;
            }
            else if (ch == '\\')
            {
                state = state::EscapedChar;
            }
            else if (ch == ' ')
            {
                // Ignore repeated runs of spaces.
                if (!arg.empty())
                {
                    args.push_back(arg);
                    arg.clear();
                }
            }
            else
            {
                arg.push_back(ch);
            }
            break;
        case state::QuotedString:
            if (ch == '"')
            {
                // End the quoted string.
                args.push_back(arg);
                arg.clear();
                state = state::Normal;
            }
            else if (ch == '\\')
            {
                state = state::EscapedQuotedString;
            }
            else
            {
                arg.push_back(ch);
            }
            break;
        case state::EscapedChar:
            arg.push_back(ch);
            state = state::Normal;
            break;
        case state::EscapedQuotedString:
            arg.push_back(ch);
            state = state::QuotedString;
            break;
        default:
            console::printf("error: Invalid command parser state.\n");
            break;
        }
    }

    // Error on incomplete lines.
    switch (state)
    {
    case state::QuotedString:
        console::printf("error: Incomplete quoted string.\n");
        return;
    case state::EscapedChar:
    case state::EscapedQuotedString:
        console::printf("error: Incomplete escape.\n");
        return;
    }

    // Add a non-empty argument as the final argument.
    if (!arg.empty())
    {
        args.push_back(arg);
    }

    // Find a command associated with argument 0.
    if (args.size() < 1)
    {
        return;
    }
    auto cmd_iter = this->command_map.find(args.at(0));
    if (cmd_iter == this->command_map.end())
    {
        console::printf("error: Command not found.\n");
        return;
    }

    // Run the command.
    cmd_iter->second(args);
}

}

}
