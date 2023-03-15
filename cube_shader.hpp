#ifndef CUBE_SHADER_HPP
#define CUBE_SHADER_HPP

#include <GL/gl.h>

struct ShaderContext
{
    GLuint program{};
    GLint modelViewProjectionMatrix{};

    GLuint vbo{};

    GLuint textureBinding{};
    GLsizei textureWidth{};
    GLsizei textureHeight{};
    GLsizei textureBpp{};
    void* textureMemory{};

    GLuint positionsOffset{};
    GLuint UVOffset{};
};

ShaderContext createCubeShader() noexcept;

void generateAndBindTexture(ShaderContext& shaderContext) noexcept;

void setDefaultTextureParameters(const ShaderContext& shaderContext) noexcept;

void drawCubeShader(const ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept;

#endif
