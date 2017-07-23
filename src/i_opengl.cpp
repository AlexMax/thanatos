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
#include "i_video.h"

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
    glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &len); // including \0

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
    if (vertexShader.Compile() == false)
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
    if (fragmentShader.Compile() == false)
    {
        I_Error("Fragment Shader Compile Error:\n%s\n", fragmentShader.Log().c_str());
    }

    // Screen Shader Program.
    this->screenProgram = std::make_unique<Program>();
    this->screenProgram->Attach(vertexShader);
    this->screenProgram->Attach(fragmentShader);
    if (this->screenProgram->Link() == false)
    {
        I_Error("Shader Program Link Error:\n%s\n", this->screenProgram->Log().c_str());
    }

    // A simple square to render the screen to.
    //
    // The first three vertices are the x,y,z locations in screen space.
    // The last two vertices are the texture coordinates.
    GLfloat vertices[][5] = {
        {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
        {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
        { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f }
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
    glGenTextures(1, &this->screenPixels);
    glBindTexture(GL_TEXTURE_2D, this->screenPixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Next, we need a second texture that contains the palettes.
    glGenTextures(1, &this->screenPalettes);
    glBindTexture(GL_TEXTURE_2D, this->screenPalettes);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Assign texture units 0 and 1 to hold our screen texture and palettes.
    glUseProgram(this->screenProgram->Ref());
    glUniform1i(glGetUniformLocation(this->screenProgram->Ref(), "uTexture"), 0);
    glUniform1i(glGetUniformLocation(this->screenProgram->Ref(), "uPalette"), 1);
}

// logs every gl call to the console.
void Renderer::debugCall(const char* name, void* function, int argc, ...) {
    printf("Renderer::debugCall: %s(", name);

    va_list args;
    va_start(args, argc);
    if (argc <= 0)
    {
        // Do nothing...
    }
    else if (function == glAttachShader)
    {
        GLuint program = va_arg(args, GLuint);
        GLuint shader = va_arg(args, GLuint);
        printf("%u, %u", program, shader);
    }
    else if (function == glLinkProgram)
    {
        GLuint program = va_arg(args, GLuint);
        printf("%u", program);
    }
    else if (function == glGetProgramiv)
    {
        GLuint program = va_arg(args, GLuint);
        GLenum pname = va_arg(args, GLenum);
        GLint* params = va_arg(args, GLint*);
        printf("%u, %u, %p", program, pname, params);
    }
    else if (function == glGetShaderiv)
    {
        GLuint shader = va_arg(args, GLuint);
        GLenum pname = va_arg(args, GLenum);
        GLint* params = va_arg(args, GLint*);
        printf("%u, %u, %p", shader, pname, params);
    }
    else
    {
        printf("...%d unknown arguments...", argc);
    }
    va_end(args);

    printf(")\n");
}

// A debug message callback called by the OpenGL driver.
void Renderer::debugMessage(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message, const void* param)
{
    printf("Renderer::debugMessage source: %u, type: %u, id:%u, severity: %u, sizei: %d\n%s\n",
        source, type, id, severity, length, message);
}

Renderer::Renderer(SDL_Window* window) :
    constructed(false), context(nullptr), window(window),
    screenProgram(nullptr), screenVAO(0), screenPixels(0), screenPalettes(0)
{
    // Do some OpenGL initialization stuff.
    this->context = SDL_GL_CreateContext(this->window);
    if (!gladLoadGL())
    {
        I_Error("gladLoadGL failed");
    }

#ifdef _DEBUG
    // Initialize driver debugging
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

#ifdef GLAD_DEBUG
    // Initialize GLAD debugging
    glad_set_pre_callback((GLADcallback)Renderer::debugCall);
#endif
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

// Flip from the backbuffer to the visible buffer.
void Renderer::Flip()
{
    SDL_GL_SwapWindow(this->window);
}

// Render the screen to the backbuffer.
void Renderer::Render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->screenPixels);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->screenPalettes);

    glUseProgram(this->screenProgram->Ref());
    glBindVertexArray(this->screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Set the current palette.
void Renderer::SetPalette(const byte* palette)
{
    glBindTexture(GL_TEXTURE_2D, this->screenPalettes);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
}

// Set the current pixel data.
void Renderer::SetPixels(const pixel_t* pixels)
{
    glBindTexture(GL_TEXTURE_2D, this->screenPixels);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, SCREENWIDTH, SCREENHEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

}

}

}
