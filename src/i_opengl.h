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
    GLuint GetShader() const;
    std::string GetLog() const;
};

// An OpenGL Shader Program
class Program
{
private:
    GLuint program;
public:
    Program() : program(glCreateProgram()) { }
    Program(const Program &obj) = delete;
    ~Program();
    void Attach(const Shader& shader);
    bool Program::Link();
    GLuint GetProgram() const;
    std::string GetLog() const;
};

class VertexArrayObject
{
private:
    GLuint vao;
public:
    VertexArrayObject();
    VertexArrayObject(const VertexArrayObject &obj) = delete;
    ~VertexArrayObject();
    GLuint Get() const;
};

class VertexBufferObject
{
private:
    GLuint vbo;
public:
    VertexBufferObject();
    VertexBufferObject(const VertexBufferObject &obj) = delete;
    ~VertexBufferObject();
    GLuint Get() const;
};

}

}

}

#endif