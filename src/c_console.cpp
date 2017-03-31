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

#include <memory>
#include <string>
#include <array>

#include "c_console.h"
#include "c_font.h"

#include "m_misc.h"
#include "w_wad.h"
#include "i_video.h"
#include "v_video.h"
#include "draw_list.h"

namespace console
{

// A single line of the console, plus its associated DrawList.
//
// FIXME: DrawLists should be invalidated on resolution switch.
struct BufferLine
{
    std::string line;
    std::unique_ptr<video::DrawList> drawer;
    BufferLine(): line(""), drawer(nullptr) { }
};

// Buffer to store past console lines.
class Buffer
{
private:
    std::array<BufferLine, 2048> line_buffer;
    size_t head;
    size_t tail;
public:
    class iterator;
    Buffer() : head(0), tail(0) { };
    static Buffer& Instance();
    void AppendLines(const char* str);
    void Clear();
    iterator begin();
    iterator end();
};

class Buffer::iterator
{
private:
    Buffer* buffer;
    size_t index;
public:
    bool operator!=(iterator &b);
    iterator& operator++();
    iterator& operator--();
    BufferLine& operator*();
    iterator(Buffer* buffer, size_t index) : buffer(buffer), index(index) { }
};

// Get the singleton instance of the class
Buffer& Buffer::Instance()
{
    static Buffer singleton;
    return singleton;
}

// Append a string to the console buffer, splitting on newlines.
void Buffer::AppendLines(const char* str)
{
    const char* sb = str;
    const char* se = str;

    auto dest = std::move(this->line_buffer.at(this->head));
    dest.drawer = nullptr;

    for(;;)
    {
        if (*se == '\n')
        {
            dest.line.append(sb, se - sb);
            this->line_buffer.at(this->head) = std::move(dest);
            this->head += 1 % this->line_buffer.size();
            if (this->head == this->tail)
            {
                this->tail += 1 % this->line_buffer.size();
            }

            dest = BufferLine();

            se += 1;
            sb = se;
        }
        else if (*se == '\0')
        {
            dest.line.append(sb, se - sb);
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
    this->line_buffer.at(0).line = "";
    this->line_buffer.at(0).drawer = nullptr;
}

// Begin iterator
Buffer::iterator Buffer::begin()
{
    return iterator(this, 0);
}

// End iterator
Buffer::iterator Buffer::end()
{
    // Our buffer always grows and never shrinks, so the only time
    // we will ever have a partway-filled buffer is if the tail
    // hasn't moved yet.
    if (tail == 0)
    {
        return iterator(this, this->head);
    }
    else
    {
        return iterator(this, this->line_buffer.size());
    }
}

bool Buffer::iterator::operator!=(iterator &b)
{
    return this->index != b.index;
}

Buffer::iterator& Buffer::iterator::operator++()
{
    this->index++;
    return *this;
}

Buffer::iterator& Buffer::iterator::operator--()
{
    this->index--;
    return *this;
}

BufferLine& Buffer::iterator::operator*()
{
    size_t index = this->buffer->tail + this->index % this->buffer->line_buffer.size();
    return this->buffer->line_buffer[index];
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
    va_list args_again;
    va_copy(args_again, args);

    int size = vsnprintf(NULL, 0, format, args) + 1;
    char* line = static_cast<char*>(malloc(size));

    vsnprintf(line, size, format, args_again);
    Buffer::Instance().AppendLines(line);

    va_end(args_again);
    free(line);
}

// Draw the console to the screen.
void Draw()
{
    V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT / 2, 0);

    auto& buffer = Buffer::Instance();
    int cony = (SCREENHEIGHT / 2) - 8;

    // Start at the end, and draw backwards.
    for (auto it = buffer.end();it != buffer.begin();)
    {
        --it;
        auto& line = *it;

        if (line.drawer == nullptr)
        {
            // We don't have a drawlist, create one.
            auto drawer = video::DrawList();
            int x = 0, y = 0, max_width = 0, max_line_height = 0;

            for (const char& c : line.line)
            {
                patch_t* patch = nullptr;
                Font::const_iterator fit = ConsoleFont.find(c);
                if (fit != ConsoleFont.end())
                {
                    auto letter = fit->second;
                    patch = const_cast<patch_t*>(reinterpret_cast<const patch_t*>(letter.GetData()));
                }
                else
                {
                    // Use space as fallback.  If that doesn't work, crash.
                    auto letter = ConsoleFont.at(32);
                    patch = const_cast<patch_t*>(reinterpret_cast<const patch_t*>(letter.GetData()));
                }

                if (patch->width + x > SCREENWIDTH)
                {
                    // Force newline.
                    x = 0;
                    y += max_line_height;
                    max_line_height = 0;
                }

                drawer.Add(V_DrawPatchDirect, patch, x, y);
                x += patch->width;

                if (x > max_width)
                {
                    max_width = x;
                }
                if (patch->height >= max_line_height)
                {
                    max_line_height = patch->height;
                }
            }

            drawer.SetWidth(max_width);
            drawer.SetHeight(y + max_line_height);

            // DrawList constructed, assign it.
            line.drawer = std::make_unique<video::DrawList>(drawer);
        }

        if (cony - line.drawer->GetHeight() < 0)
        {
            // Don't draw off the top of the screen.
            break;
        }

        line.drawer->Draw(0, cony - line.drawer->GetHeight());
        cony -= line.drawer->GetHeight();
    }
}

}
