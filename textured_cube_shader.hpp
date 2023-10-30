#ifndef KZ_CUBE_SHADER_HPP
#define KZ_CUBE_SHADER_HPP

#include "gl_functions.h"

struct Matrix4x4
{
    GLfloat* operator[](size_t index) noexcept
    {
        return data[index];
    }
    GLfloat data[4][4];
};

// TODO: Split these.
// TODO: Cleanup.
struct ShaderContext
{
    GLuint cubeProgram{};
    GLuint rttProgram{};

    GLint modelViewProjectionMatrixUniform{};
    GLint uvRepeatCountUniform{};
    GLint objectIDUniform{};
    GLint drawIDUniform{};
    GLint subPixelResolutionUniform{};

    Matrix4x4 modelViewProjection{};

    GLfloat uvRepeatCount{};

    GLuint cubeVAO{};
    GLuint cubePickingVAO{};

    GLuint cubeVBO{};
    GLuint cubePickingVBO{};

    GLuint textureBinding{};
    GLsizei textureWidth{};
    GLsizei textureHeight{};
    GLsizei textureBpp{};
    const void* textureMemory{};

    GLuint positionsOffset{};
    GLuint UVOffset{};
};

ShaderContext createCubeShader() noexcept;

void generateAndBindTexture(ShaderContext& shaderContext) noexcept;

void generateAndBindDefaultTexture(ShaderContext& shaderContext) noexcept;

void setDefaultGLTextureParameters(const ShaderContext& shaderContext) noexcept;

void setupCubeShaderView(ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept;

void drawCubeShaderToTexture(ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept;

void drawTexturedCubeShaderToOutput(ShaderContext& context, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept;

void drawCubeShaderElapsed(const ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int usElapsed) noexcept;

#endif
