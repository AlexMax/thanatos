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

#ifndef __C_COMMANDS__
#define __C_COMMANDS__

#include <functional>
#include <string>
#include <unordered_map>

namespace console
{

typedef std::vector<std::string> CommandArguments;

typedef std::function<void(CommandArguments)> CommandImplementation;

class Commands
{
    std::unordered_map<const char*, CommandImplementation> command_map;
public:
    static Commands& Instance();
    void Add(const char* name, CommandImplementation func);
    void Execute(const std::string& line);
};

}

#endif