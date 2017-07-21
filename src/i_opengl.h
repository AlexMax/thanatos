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
//     OpenGL helper classes.
//

#ifndef __I_OPENGL__
#define __I_OPENGL__

#include <string>

#include "SDL.h"
#include "glad/glad.h"

namespace theta
{

namespace system
{

namespace gl
{

// An OpenGL shader.
class Shader
{
private:
    GLuint shader;
public:
    enum class type {
        vertex,
        fragment
    };
    Shader(type);
    Shader(const Shader &obj) = delete;
    ~Shader();
    void Source(const char* source);
    bool Compile();
    GLuint Ref() const;
    std::string Log() const;
};

// An OpenGL Shader Program
class Program
{
private:
    GLuint program;
public:
    Program();
    Program(const Program &obj) = delete;
    ~Program();
    void Attach(const Shader& shader);
    bool Program::Link();
    GLuint Ref() const;
    std::string Log() const;
};

class Renderer
{
private:
    bool constructed;
    SDL_GLContext context;
    GLuint screenProgram;
    GLuint screenVAO;
    GLuint screenPixels;
    GLuint screenPalettes;
    void constructScreen();
    static void debugMessage(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length, const GLchar* message, const void* param);
public:
    Renderer(SDL_Window* window);
    Renderer(const Renderer&) = delete;
    Renderer::~Renderer();
    void Render();
    void Renderer::Flip(SDL_Window* window);
};

}

}

}

#endif