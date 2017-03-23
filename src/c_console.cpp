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
//     Console.
//

#include <string>
#include <array>

#include "c_console.h"

#include "m_misc.h"

namespace console
{

// Buffer to store past console lines.
class Buffer
{
private:
    std::array<std::string, 2048> line_buffer;
    size_t head;
    size_t tail;
public:
    Buffer() : head(0), tail(0) { };
    static Buffer& Buffer::Instance();
    void AppendLines(const char* str);
    void Clear();
};

// Get the singleton instance of the class
Buffer& Buffer::Instance()
{
    static Buffer singleoton;
    return singleoton;
}

// Append a string to the console buffer, splitting on newlines.
void Buffer::AppendLines(const char* str)
{
    const char* sb = str;
    const char* se = str;

    std::string dest = std::move(this->line_buffer.at(this->head));

    for(;;)
    {
        if (*se == '\n')
        {
            dest.append(sb, se - sb);
            this->line_buffer.at(this->head) = std::move(dest);
            this->head += 1 % this->line_buffer.size();
            if (this->head == this->tail)
            {
                this->tail += 1 % this->line_buffer.size();
            }

            dest = std::string();

            se += 1;
            sb = se;
        }
        else if (*se == '\0')
        {
            dest.append(sb, se - sb);
            this->line_buffer.at(this->head) = std::move(dest);
            break;
        }
        else
        {
            se += 1;
        }
    }
}

// Clear (for all intents and purposes) the console buffer.
void Buffer::Clear()
{
    this->head = 0;
    this->tail = 0;
    this->line_buffer.at(0) = "";
}

// Append the printed string to the console buffer.
void printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    console::vprintf(format, args);
    va_end(args);
}

// Append the printed string to the console buffer.
void vprintf(const char* format, va_list args)
{
    // WARNING: This function relies upon a C99-conforming vsnprintf().  Do
    //          not replace it with _vsnprintf or M_vsnprintf.
    va_list args_again = va_copy(args_again, args);

    int size = vsnprintf(NULL, 0, format, args) + 1;
    char* line = static_cast<char*>(malloc(size));

    vsnprintf(line, size, format, args_again);
    Buffer::Instance().AppendLines(line);

    va_end(args_again);
    free(line);
}

}