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

// Constructs the graphics renderer, which handles batched 2D graphics
// rendering.
void Renderer::constructGraphics()
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
        "in vec2 testTexCoord;\n\n"

        "uniform sampler2D uTexture;\n\n"

        "void main()\n"
        "{\n"
        "    oFragColor = texture(uTexture, testTexCoord);\n"
        "}\n";
    Shader fragmentShader(Shader::type::fragment);
    fragmentShader.Source(fragmentShaderSource);
    if (fragmentShader.Compile() == false)
    {
        I_Error("Fragment Shader Compile Error:\n%s\n", fragmentShader.Log().c_str());
    }

    // Page Shader Program.
    this->graphicsProgram = std::make_unique<Program>();
    this->graphicsProgram->Attach(vertexShader);
    this->graphicsProgram->Attach(fragmentShader);
    if (this->graphicsProgram->Link() == false)
    {
        I_Error("Shader Program Link Error:\n%s\n", this->graphicsProgram->Log().c_str());
    }

    // Bind the square to a VAO containing a VBO and our IBO.
    glGenVertexArrays(1, &this->graphicsVAO);
    glBindVertexArray(this->graphicsVAO);

    glGenBuffers(1, &this->graphicsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->graphicsVBO);

    glGenBuffers(1, &this->graphicsIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->graphicsIBO);

    // location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    // Location 1
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Unbind our VBO, IBO and VAO so we don't accidentally modify them.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // All we need is a texture that contains the RGBA graphics data.
    glGenTextures(1, &this->graphicsPixels);
    glBindTexture(GL_TEXTURE_2D, this->graphicsPixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create an empty texture, since we're just using this as an atlas.
    GLuint atlas_size = 2048;
    std::vector<GLuint> blank(atlas_size * atlas_size * 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlas_size, atlas_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, blank.data());

    // Assign texture unit 0 to hold our page.
    glUseProgram(this->graphicsProgram->Ref());
    glUniform1i(glGetUniformLocation(this->graphicsProgram->Ref(), "uTexture"), 0);

    // Set up the graphics atlas of the same size.
    this->graphicsAtlas = std::make_unique<video::Atlas>(atlas_size, atlas_size);
}

// Constructs the page, the target of any full-screen 2D renderings like
// the title screen or the help screen.
void Renderer::constructPage()
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
        "in vec2 testTexCoord;\n\n"

        "uniform sampler2D uTexture;\n\n"

        "void main()\n"
        "{\n"
        "    oFragColor = texture(uTexture, testTexCoord);\n"
        "}\n";
    Shader fragmentShader(Shader::type::fragment);
    fragmentShader.Source(fragmentShaderSource);
    if (fragmentShader.Compile() == false)
    {
        I_Error("Fragment Shader Compile Error:\n%s\n", fragmentShader.Log().c_str());
    }

    // Page Shader Program.
    this->pageProgram = std::make_unique<Program>();
    this->pageProgram->Attach(vertexShader);
    this->pageProgram->Attach(fragmentShader);
    if (this->pageProgram->Link() == false)
    {
        I_Error("Shader Program Link Error:\n%s\n", this->pageProgram->Log().c_str());
    }

    // A simple square to render the screen to.
    //
    // The first three vertices are the x,y,z locations in screen space.
    // The last two vertices are the texture coordinates.
    GLfloat vertices[][5] = {
        { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
        { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
        { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f }
    };

    // Bind the square to a VAO containing a VBO.
    glGenVertexArrays(1, &this->pageVAO);
    glBindVertexArray(this->pageVAO);

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

    // All we need is a texture that contains the RGBA page data.
    glGenTextures(1, &this->pagePixels);
    glBindTexture(GL_TEXTURE_2D, this->pagePixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Assign texture unit 0 to hold our page.
    glUseProgram(this->pageProgram->Ref());
    glUniform1i(glGetUniformLocation(this->pageProgram->Ref(), "uTexture"), 0);
}

// Constructs the "world", the main target of the 3D renderer.
void Renderer::constructWorld()
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

    // World Shader Program.
    this->worldProgram = std::make_unique<Program>();
    this->worldProgram->Attach(vertexShader);
    this->worldProgram->Attach(fragmentShader);
    if (this->worldProgram->Link() == false)
    {
        I_Error("Shader Program Link Error:\n%s\n", this->worldProgram->Log().c_str());
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
    glGenVertexArrays(1, &this->worldVAO);
    glBindVertexArray(this->worldVAO);

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
    glGenTextures(1, &this->worldPixels);
    glBindTexture(GL_TEXTURE_2D, this->worldPixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Next, we need a second texture that contains the palettes.
    glGenTextures(1, &this->worldPalettes);
    glBindTexture(GL_TEXTURE_2D, this->worldPalettes);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Assign texture units 0 and 1 to hold our screen texture and palettes.
    glUseProgram(this->worldProgram->Ref());
    glUniform1i(glGetUniformLocation(this->worldProgram->Ref(), "uTexture"), 0);
    glUniform1i(glGetUniformLocation(this->worldProgram->Ref(), "uPalette"), 1);
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
    constructed(false), context(nullptr), maxTextureSize(0),
    renderSource(renderSources::none), viewportHeight(0), viewportWidth(0),
    window(window),
    graphicsAtlas(nullptr), graphicsIBO(0), graphicsIndices(0), graphicsPixels(0),
    graphicsProgram(nullptr), graphicsVAO(0), graphicsVBO(0), graphicsVertices(0),
    pagePixels(0), pageProgram(nullptr), pageVAO(0),
    worldPixels(0), worldPalettes(0), worldProgram(nullptr), worldVAO(0)
{
    // Do some OpenGL initialization stuff.  We want a core profile.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    this->context = SDL_GL_CreateContext(this->window);
    if (this->context == NULL)
    {
        I_Error("Context creation failed: %s\n", SDL_GetError());
    }

    if (!gladLoadGL())
    {
        I_Error("gladLoadGL failed");
    }

#if defined(_DEBUG) && defined(BREAK_GLASS)
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

    // Ensure our viewport width and height are correct.
    SDL_GetWindowSize(window, &this->viewportWidth, &this->viewportHeight);
    glViewport(0, 0, this->viewportWidth, this->viewportHeight);

    // Backface culling
    glEnable(GL_CULL_FACE);

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Align on byte boundaries.  Without this, our pixel data will skew
    // when the resolution is not evenly divisible by four.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Maximum texture size governs how big our resolution can be, as well
    // how big our texture atlases can be.  2048 is a reasonable minimum,
    // since without that we can't render a 1080p screen.
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->maxTextureSize);
    if (this->maxTextureSize < 2048)
    {
        I_Error("Maximum texture size is too small (needs 2048, found %d).", this->maxTextureSize);
    }

    // Set up the graphics overlay view.
    this->constructGraphics();

    // Set up the world view.
    this->constructWorld();

    // Set up the page view.
    this->constructPage();

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

    if (this->renderSource == renderSources::world)
    {
        // Render the 3D world.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->worldPixels);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, this->worldPalettes);

        glUseProgram(this->worldProgram->Ref());
        glBindVertexArray(this->worldVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else if (this->renderSource == renderSources::page)
    {
        // Render a 2D page.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->pagePixels);

        glUseProgram(this->pageProgram->Ref());
        glBindVertexArray(this->pageVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else
    {
        I_Error("No renderSource defined.");
    }

    // Draw 2D graphics on top of that.
    if (this->graphicsVertices.size() > 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->graphicsPixels);

        glUseProgram(this->graphicsProgram->Ref());
        glBindVertexArray(this->graphicsVAO);

        glBindBuffer(GL_ARRAY_BUFFER, this->graphicsVBO);
        glBufferData(GL_ARRAY_BUFFER,
            this->graphicsVertices.size() * sizeof(decltype(this->graphicsVertices)::value_type),
            this->graphicsVertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            this->graphicsIndices.size() * sizeof(decltype(this->graphicsIndices)::value_type),
            this->graphicsIndices.data(), GL_STATIC_DRAW);

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->graphicsIndices.size()),
            GL_UNSIGNED_INT, 0);

        this->graphicsVertices.clear();
        this->graphicsIndices.clear();
    }

    // Render the world by default, you must force pages by hand.
    this->renderSource = renderSources::world;
}

// Add graphic to the texture atlas.
void Renderer::AddGraphic(const std::string& name, const video::RGBABuffer& pixels, int xoff, int yoff)
{
    this->graphicsAtlas->Add(name, pixels.GetWidth(), pixels.GetHeight(), xoff, yoff);

    video::AtlasEntry atlas(0, 0, 0, 0, 0, 0);
    if (this->graphicsAtlas->Find(name, atlas) == false)
    {
        return;
    }

    // Use the atlas entry to draw our graphic into the atlas texture.
    glBindTexture(GL_TEXTURE_2D, this->graphicsPixels);
    glTexSubImage2D(GL_TEXTURE_2D, 0, atlas.x, atlas.y, atlas.w, atlas.h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.GetRawPixels());
}

// Check to see if a graphic exists in the texture atlas.
bool Renderer::CheckGraphic(const std::string& name)
{
    return this->graphicsAtlas->Check(name);
}

// Draw a graphic that should exist in the texture atlas.
void Renderer::DrawGraphic(const std::string& name, int x, int y, double scalex, double scaley)
{
    video::AtlasEntry atlas(0, 0, 0, 0, 0, 0);
    if (this->graphicsAtlas->Find(name, atlas) == false)
    {
        return;
    }

    // Doom uses X, Y coordinates that begin in the upper left, but OpenGL
    // has a floating point graph that goes from -1, -1 to 1, 1 starting
    // at the lower left.  Convert between the coordinate systems.
    GLfloat x1 = static_cast<GLfloat>((2 * scalex * (atlas.xoff + x)) / static_cast<double>(this->viewportWidth) - 1);
    GLfloat y1 = static_cast<GLfloat>(1 - ((2 * scaley * (atlas.yoff + y)) / static_cast<double>(this->viewportHeight)));
    GLfloat u1 = atlas.x / (GLfloat)this->graphicsAtlas->GetWidth();
    GLfloat v1 = atlas.y / (GLfloat)this->graphicsAtlas->GetHeight();

    GLfloat x2 = static_cast<GLfloat>((2 * scalex * (atlas.xoff + atlas.w + x)) / static_cast<double>(this->viewportWidth) - 1);
    GLfloat y2 = static_cast<GLfloat>(1 - ((2 * scaley * (atlas.yoff + atlas.h + y)) / static_cast<double>(this->viewportHeight)));
    GLfloat u2 = (atlas.x + atlas.w) / (GLfloat)this->graphicsAtlas->GetWidth();
    GLfloat v2 = (atlas.y + atlas.h) / (GLfloat)this->graphicsAtlas->GetHeight();

    GLfloat z = 0.0f;

    // Top left
    this->graphicsVertices.emplace_back(x1);
    this->graphicsVertices.emplace_back(y1);
    this->graphicsVertices.emplace_back(z);
    this->graphicsVertices.emplace_back(u1);
    this->graphicsVertices.emplace_back(v1);

    // Top right
    this->graphicsVertices.emplace_back(x2);
    this->graphicsVertices.emplace_back(y1);
    this->graphicsVertices.emplace_back(z);
    this->graphicsVertices.emplace_back(u2);
    this->graphicsVertices.emplace_back(v1);

    // Bottom left
    this->graphicsVertices.emplace_back(x1);
    this->graphicsVertices.emplace_back(y2);
    this->graphicsVertices.emplace_back(z);
    this->graphicsVertices.emplace_back(u1);
    this->graphicsVertices.emplace_back(v2);

    // Bottom right
    this->graphicsVertices.emplace_back(x2);
    this->graphicsVertices.emplace_back(y2);
    this->graphicsVertices.emplace_back(z);
    this->graphicsVertices.emplace_back(u2);
    this->graphicsVertices.emplace_back(v2);

    // Indices.
    GLuint offset = static_cast<GLuint>((this->graphicsVertices.size() / 20 - 1) * 4);
    this->graphicsIndices.emplace_back(offset + 3);
    this->graphicsIndices.emplace_back(offset + 1);
    this->graphicsIndices.emplace_back(offset + 2);
    this->graphicsIndices.emplace_back(offset + 0);
    this->graphicsIndices.emplace_back(offset + 2);
    this->graphicsIndices.emplace_back(offset + 1);
}

// Get the height of the viewport.
int Renderer::GetHeight() const
{
    return this->viewportHeight;
}

// Get the width of the viewport.
int Renderer::GetWidth() const
{
    return this->viewportWidth;
}


// Set pixel data for a full screen "page".
void Renderer::SetPagePixels(const video::RGBABuffer& pixels)
{
    this->renderSource = renderSources::page;
    glBindTexture(GL_TEXTURE_2D, this->pagePixels);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pixels.GetWidth(), pixels.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.GetRawPixels());
}

// Nudge OpenGL to change the size of the viewport to match the new resolution.
void Renderer::SetResolution(int width, int height)
{
    glViewport(0, 0, width, height);
    this->viewportWidth = width;
    this->viewportHeight = height;
}

// Set the current palette for the 3D rendered world view.
void Renderer::SetWorldPalette(const byte* palette)
{
    glBindTexture(GL_TEXTURE_2D, this->worldPalettes);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
}

// Set the current pixel data for the 3D rendered world view.
void Renderer::SetWorldPixels(const video::PalletedBuffer& pixels)
{
    glBindTexture(GL_TEXTURE_2D, this->worldPixels);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, pixels.GetWidth(), pixels.GetHeight(), 0, GL_RED, GL_UNSIGNED_BYTE, pixels.GetRawPixels());
}

}

}

}
