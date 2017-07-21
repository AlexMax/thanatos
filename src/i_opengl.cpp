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

#include <cstdlib>

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
    GLint len;
    glGetShaderiv(this->program, GL_INFO_LOG_LENGTH, &len); // including \0

    if (len > 0)
    {
        char* buffer = static_cast<char*>(std::malloc(len));
        glGetProgramInfoLog(this->program, sizeof(buffer), NULL, buffer);
        return std::string(buffer);
    }
    else
    {
        return std::string();
    }
}

// Constructs the primary screen, the main target of the 3D renderer.
void Renderer::constructScreen()
{
    if (this->constructed)
    {
        I_Error("Something called Renderer::constructScreen twice");
    }

    // Vertex Shader
    const char* vertexShaderSource =
        "#version 330 core\n\n"

        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n\n"

        "out vec2 testTexCoord;\n\n"

        "void main()\n"
        "{\n"
        "    gl_Position = vec4(aPos, 1.0);\n"
        "    testTexCoord = aTexCoord;\n"
        "}";
    Shader vertexShader(Shader::type::vertex);
    vertexShader.Source(vertexShaderSource);
    if (!vertexShader.Compile())
    {
        I_Error("Vertex Shader Compile Error:\n%s\n", vertexShader.Log().c_str());
    }

    // Fragment Shader
    const char* fragmentShaderSource =
        "#version 330 core\n\n"

        "out vec4 oFragColor;\n"
        "in vec3 testColor;\n"
        "in vec2 testTexCoord;\n\n"

        "uniform sampler2D uTexture;\n"
        "uniform sampler2D uPalette;\n\n"

        "void main()\n"
        "{\n"
        "    vec4 index = texture(uTexture, testTexCoord);\n"
        "    oFragColor = texture(uPalette, vec2(index.x, 0.0));\n"
        "}\n";
    Shader fragmentShader(Shader::type::fragment);
    fragmentShader.Source(fragmentShaderSource);
    if (!fragmentShader.Compile())
    {
        I_Error("Fragment Shader Compile Error:\n%s\n", fragmentShader.Log().c_str());
    }

    // Screen Shader Program
    // FIXME: For some reason, having this class as part of the parent context
    //        class just plain doesn't work - at least not my first attempt.
    Program* _screenProgram = new Program();
    _screenProgram->Attach(vertexShader);
    _screenProgram->Attach(fragmentShader);
    if (!_screenProgram->Link())
    {
        I_Error("Shader Program Link Error:\n%s\n", _screenProgram->Log().c_str());
    }
    this->screenProgram = _screenProgram->Ref();

    // A simple square to render the screen to.
    //
    // The first three vertices are the x,y,z locations in screen space.
    // The last two vertices are the texture coordinates.
    GLfloat vertices[][5] = {
        {  1.0f, -1.0f, 0.0f, 1.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f, 1.0f, 1.0f },
        { -1.0f, -1.0f, 0.0f, 0.0f, 0.0f },
        { -1.0f,  1.0f, 0.0f, 0.0f, 1.0f }
    };

    // Bind the square to a VAO containing a VBO.
    glGenVertexArrays(1, &this->screenVAO);
    glBindVertexArray(this->screenVAO);

    GLuint VBOnum;
    glGenBuffers(1, &VBOnum);
    glBindBuffer(GL_ARRAY_BUFFER, VBOnum);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    // Location 1
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Unbind our VBO and VAO so we don't accidentally modify them.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize our screen textures.

    // First, we need a texture that contains the raw 8-bit screen data.
    GLubyte screenPixelData[320 * 200] = { 0 };
    for (int i = 0;i < sizeof(screenPixelData);i++)
    {
        screenPixelData[i] = rand() % 0x100;
    }

    glGenTextures(1, &this->screenPixels);
    glBindTexture(GL_TEXTURE_2D, this->screenPixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 320, 200, 0, GL_RED, GL_UNSIGNED_BYTE, screenPixelData);

    // Next, we need a second texture that contains the palettes.
    GLubyte paletteTextureData[256][3] = { 0 };
    for (int i = 0;i < 256;i++)
    {
        paletteTextureData[i][0] = 255 - i; // red
        paletteTextureData[i][1] = 0; // green
        paletteTextureData[i][2] = i; // hot pink....nah, just blue
    }

    glGenTextures(1, &this->screenPalettes);
    glBindTexture(GL_TEXTURE_2D, this->screenPalettes);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, paletteTextureData);

    // Assign texture units 0 and 1 to hold our screen texture and palettes.
    glUseProgram(this->screenProgram);
    glUniform1i(glGetUniformLocation(this->screenProgram, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(this->screenProgram, "uPalette"), 1);
}

// A debug message callback called by the OpenGL driver.
void Renderer::debugMessage(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
    printf("OGL source: %u, type: %u, id:%u, severity: %u, sizei: %d\n%s\n",
        source, type, id, severity, length, message);
}

Renderer::Renderer(SDL_Window* window) :
    constructed(false), context(nullptr),
    screenProgram(0), screenVAO(0), screenPixels(0), screenPalettes(0)
{
    this->context = SDL_GL_CreateContext(window);

#ifdef _DEBUG
    if (GL_KHR_debug)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROC)Renderer::debugMessage, NULL);
        glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true
        );
    }
    else
    {
        I_Error("Your OpenGL implementation does not support GL_KHR_debug");
    }
#endif

    // Basic initialization
    glEnable(GL_CULL_FACE);

    // Set up the screen.
    this->constructScreen();

    // Prevent nefarious people from calling constructor-only methods.
    this->constructed = true;
}

Renderer::~Renderer()
{
    SDL_GL_DeleteContext(this->context);
}

// Actually render the screen.
void Renderer::Render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->screenPixels);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->screenPalettes);

    glUseProgram(this->screenProgram);
    glBindVertexArray(this->screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Flip from the backbuffer to the visible buffer.
void Renderer::Flip(SDL_Window* window)
{
    SDL_GL_SwapWindow(window);
}

}

}

}
