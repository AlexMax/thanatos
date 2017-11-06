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
//     File to include file converter.  This code is not great or
//     fast, merely "good enough".
//

#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// Determine variable name given an input path
std::string VariableName(const std::string& path)
{
#if _WIN32
    auto index = path.find_last_of("/\\");
#else
    auto index = path.find_last_of("/");
#endif
    if (index == std::string::npos)
    {
        index = 0;
    }
    else
    {
        index += 1;
    }

    std::size_t len = 0;
    for (auto it = path.begin() + index;it != path.end();++it)
    {
        if (std::isalnum(*it))
        {
            len += 1;
        }
        else
        {
            break;
        }
    }

    return path.substr(index, len);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::fprintf(stderr, "requires two arguments <outfile> <infile>\n");
        exit(EXIT_FAILURE);
    }

    auto infile = std::fopen(argv[2], "rb");
    if (infile == nullptr)
    {
        std::fprintf(stderr, "can't open input file %s (%s)\n", argv[2], std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    auto basefile = VariableName(argv[2]);
    if (basefile.empty())
    {
        std::fprintf(stderr, "filename %s does not create a valid identifier\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    auto outfile = std::fopen(argv[1], "wb");
    if (outfile == nullptr)
    {
        std::fprintf(stderr, "can't open output file %s (%s)\n", argv[1], std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    // We need the complete size of the file ahead of time
    if (std::fseek(infile, 0, SEEK_END) != 0)
    {
        std::fprintf(stderr, "can't seek file %s (%s)\n", argv[2], std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    auto insize = std::ftell(infile);
    if (insize == -1L)
    {
        std::fprintf(stderr, "can't tell file %s (%s)\n", argv[2], std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (std::fseek(infile, 0, SEEK_SET) != 0)
    {
        std::fprintf(stderr, "can't seek file %s (%s)\n", argv[2], std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Write the header
    auto bytes = std::fprintf(outfile, "std::array<unsigned char, %d> %s{\n", insize, basefile.c_str());
    if (bytes < 0)
    {
        std::fprintf(stderr, "can't write file %s (%s)\n", argv[1], std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    auto buffer = std::array<unsigned char, 1024>();
    std::size_t global_index = 0;
    for (;;)
    {
        auto count = std::fread(buffer.data(), sizeof(unsigned char), buffer.size(), infile);
        for (std::size_t index = 0;index < count;index++,global_index++)
        {
            auto pos = global_index % 12;
            if (global_index == 0)
            {
                // Beginning of file
                bytes = std::fprintf(outfile, "\t0x%X", buffer[index]);
                if (bytes < 0)
                {
                    std::fprintf(stderr, "can't write file %s (%s)\n", argv[1], std::strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            else if (pos % 12 == 0)
            {
                // Beginning of a row
                bytes = std::fprintf(outfile, ",\n\t0x%X", buffer[index]);
                if (bytes < 0)
                {
                    std::fprintf(stderr, "can't write file %s (%s)\n", argv[1], std::strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                bytes = std::fprintf(outfile, ", 0x%X", buffer[index]);
                if (bytes < 0)
                {
                    std::fprintf(stderr, "can't write file %s (%s)\n", argv[1], std::strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
        }

        if (count != buffer.size())
        {
            if (std::feof(infile) != 0)
            {
                break;
            }
            else if (std::ferror(infile) != 0)
            {
                std::fprintf(stderr, "can't read file %s\n", argv[2]);
                exit(EXIT_FAILURE);
            }
            else
            {
                std::fprintf(stderr, "unknown partial read condition\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    bytes = std::fprintf(outfile, "\n};");
    if (bytes < 0)
    {
        std::fprintf(stderr, "can't write file %s (%s)\n", argv[1], std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "wrote %s\n", argv[1]);
    exit(EXIT_SUCCESS);
}