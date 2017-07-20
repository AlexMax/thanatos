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

#include "i_opengl.h"

#include "i_system.h"

namespace theta
{

namespace system
{

namespace gl
{

Shader::Shader(type type)
{
    switch (type)
    {
    case Shader::type::vertex:
        this->shader = glCreateShader(GL_VERTEX_SHADER);
        break;
    case Shader::type::fragment:
        this->shader = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    default:
        I_Error("Unknown shader type");
    }

    if (this->shader == 0)
    {
        I_Error("Could not create shader");
    }
}

Shader::~Shader()
{
    glDeleteShader(this->shader);
}

// Set the shader source.
void Shader::Source(const char* source)
{
    glShaderSource(this->shader, 1, &source, NULL);
}

// Compile a shader.  Returns true if successful, otherwise false.
bool Shader::Compile()
{
    glCompileShader(this->shader);

    // Check for compile errors.
    GLint ok;
    glGetShaderiv(this->shader, GL_COMPILE_STATUS, &ok);

    return ok == GL_TRUE;
}

// Get the shader ID.
GLuint Shader::Ref() const
{
    return this->shader;
}

// Get the last reported error, if any.
std::string Shader::Log() const
{
    char buffer[8192];
    glGetShaderInfoLog(this->shader, sizeof(buffer), NULL, buffer);
    return std::string(buffer);
}

Program::Program() : program(glCreateProgram())
{
    if (this->program == 0)
    {
        I_Error("Could not create program");
    }
}

Program::~Program()
{
    glDeleteProgram(this->program);
}

// Attach a shader to the shader program.
void Program::Attach(const Shader& shader)
{
    glAttachShader(this->program, shader.Ref());
}

// Link the shader program.
bool Program::Link()
{
    glLinkProgram(this->program);

    // Check for link errors.
    GLint ok;
    glGetProgramiv(this->program, GL_LINK_STATUS, &ok);

    return ok == GL_TRUE;
}

// Get the program ID.
GLuint Program::Ref() const
{
    return this->program;
}

// Get the last reported error, if any.
std::string Program::Log() const
{
    char buffer[8192];
    glGetProgramInfoLog(this->program, sizeof(buffer), NULL, buffer);
    return std::string(buffer);
}

}

}

}
