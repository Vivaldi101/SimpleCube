#include "assert.h"

#include <cube_shader.hpp>

#include <cmath>
#include <string>

#define haltWithMessage(msg) \
    fprintf(stderr, (msg));  \
    abort(0)

namespace
{
constexpr float PI{ 3.1415926535897932384626433832795f };

constexpr GLfloat cubeVertices[] = {
    // Front face.

    +1.0f,
    -1.0f,
    +1.0f,

    +1.0f,
    +1.0f,
    +1.0f,

    -1.0f,
    -1.0f,
    +1.0f,

    -1.0f,
    +1.0f,
    +1.0f,

    // Back face.

    -1.0f,
    -1.0f,
    -1.0f,

    -1.0f,
    +1.0f,
    -1.0f,

    +1.0f,
    -1.0f,
    -1.0f,

    +1.0f,
    +1.0f,
    -1.0f,

    // Right face.

    +1.0f,
    -1.0f,
    -1.0f,

    +1.0f,
    +1.0f,
    -1.0f,

    +1.0f,
    -1.0f,
    +1.0f,

    +1.0f,
    +1.0f,
    +1.0f,

    // Left face.

    -1.0f,
    -1.0f,
    +1.0f,

    -1.0f,
    +1.0f,
    +1.0f,

    -1.0f,
    -1.0f,
    -1.0f,

    -1.0f,
    +1.0f,
    -1.0f,

    // Top face.

    +1.0f,
    +1.0f,
    +1.0f,

    +1.0f,
    +1.0f,
    -1.0f,

    -1.0f,
    +1.0f,
    +1.0f,

    -1.0f,
    +1.0f,
    -1.0f,

    // Bottom face.

    +1.0f,
    -1.0f,
    -1.0f,

    +1.0f,
    -1.0f,
    +1.0f,

    -1.0f,
    -1.0f,
    -1.0f,

    -1.0f,
    -1.0f,
    +1.0f,
};

struct CubeFaceUVCoordinates
{
    const float bottomRight[2]{ 1.0f, 0.0f };
    const float topRight[2]{ 1.0f, 1.0f };
    const float bottomleft[2]{ 0.0f, 0.0f };
    const float topLeft[2]{ 0.0f, 1.0f };
};

constexpr CubeFaceUVCoordinates cubeUVs[6];

struct Matrix4x4
{
    GLfloat* operator[](size_t index) noexcept
    {
        return data[index];
    }
    GLfloat data[4][4];
};

Matrix4x4 getIdentityMatrix() noexcept
{
    Matrix4x4 result{};

    result.data[0][0] = 1.0f;
    result.data[1][1] = 1.0f;
    result.data[2][2] = 1.0f;
    result.data[3][3] = 1.0f;

    return result;
}

Matrix4x4 matrixMultiply(const Matrix4x4& left, const Matrix4x4& right) noexcept
{
    Matrix4x4 result{};
    for (int i = 0; i < 4; ++i)
    {
        result.data[i][0] = (left.data[i][0] * right.data[0][0]) +
                            (left.data[i][1] * right.data[1][0]) +
                            (left.data[i][2] * right.data[2][0]) +
                            (left.data[i][3] * right.data[3][0]);

        result.data[i][1] = (left.data[i][0] * right.data[0][1]) +
                            (left.data[i][1] * right.data[1][1]) +
                            (left.data[i][2] * right.data[2][1]) +
                            (left.data[i][3] * right.data[3][1]);

        result.data[i][2] = (left.data[i][0] * right.data[0][2]) +
                            (left.data[i][1] * right.data[1][2]) +
                            (left.data[i][2] * right.data[2][2]) +
                            (left.data[i][3] * right.data[3][2]);

        result.data[i][3] = (left.data[i][0] * right.data[0][3]) +
                            (left.data[i][1] * right.data[1][3]) +
                            (left.data[i][2] * right.data[2][3]) +
                            (left.data[i][3] * right.data[3][3]);
    }

    return result;
}

Matrix4x4 getProjectionMatrix(float left, float right, float bottom, float top, float nearZ, float farZ) noexcept
{
    const float deltaX = right - left;
    const float deltaY = top - bottom;
    const float deltaZ = farZ - nearZ;

    Matrix4x4 identity = getIdentityMatrix();
    Matrix4x4 result = getIdentityMatrix();

    result.data[0][0] = 2.0f * nearZ / deltaX;
    result.data[0][1] = result.data[0][2] = result.data[0][3] = 0.0f;

    result.data[1][1] = 2.0f * nearZ / deltaY;
    result.data[1][0] = result.data[1][2] = result.data[1][3] = 0.0f;

    result.data[2][0] = (right + left) / deltaX;
    result.data[2][1] = (top + bottom) / deltaY;
    result.data[2][2] = -(nearZ + farZ) / deltaZ;
    result.data[2][3] = -1.0f;

    result.data[3][2] = -2.0f * nearZ * farZ / deltaZ;
    result.data[3][0] = result.data[3][1] = result.data[3][3] = 0.0f;

    result = matrixMultiply(identity, result);

    return result;
}

Matrix4x4 getTranslatedMatrix(const Matrix4x4& matrix, float tx, float ty, float tz) noexcept
{
    Matrix4x4 result = matrix;

    result[3][0] = tx;
    result[3][1] = ty;
    result[3][2] = tz;
    result[3][3] = 1.0f;

    return result;
}

Matrix4x4 getScaledMatrix(const Matrix4x4& matrix, float sx, float sy, float sz) noexcept
{
    Matrix4x4 result = matrix;

    result[0][0] *= sx;
    result[1][0] *= sx;
    result[2][0] *= sx;

    result[0][1] *= sy;
    result[1][1] *= sy;
    result[2][1] *= sy;

    result[0][2] *= sz;
    result[1][2] *= sz;
    result[2][2] *= sz;

    return result;
}

Matrix4x4 getAxisRotatedMatrix(const Matrix4x4& matrix, float angle, float x, float y, float z) noexcept
{
    Matrix4x4 result = matrix;

    // To radians.
    float sinAngle = std::sin(angle * PI / 180.0f);
    float cosAngle = std::cos(angle * PI / 180.0f);
    float mag = std::sqrt(x * x + y * y + z * z);

    if (mag > 0.0f)
    {
        GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs;
        GLfloat oneMinusCos;
        Matrix4x4 rotationTransform;

        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        oneMinusCos = 1.0f - cosAngle;

        rotationTransform.data[0][0] = (oneMinusCos * xx) + cosAngle;
        rotationTransform.data[0][1] = (oneMinusCos * xy) - zs;
        rotationTransform.data[0][2] = (oneMinusCos * zx) + ys;
        rotationTransform.data[0][3] = 0.0f;

        rotationTransform.data[1][0] = (oneMinusCos * xy) + zs;
        rotationTransform.data[1][1] = (oneMinusCos * yy) + cosAngle;
        rotationTransform.data[1][2] = (oneMinusCos * yz) - xs;
        rotationTransform.data[1][3] = 0.0f;

        rotationTransform.data[2][0] = (oneMinusCos * zx) - ys;
        rotationTransform.data[2][1] = (oneMinusCos * yz) + xs;
        rotationTransform.data[2][2] = (oneMinusCos * zz) + cosAngle;
        rotationTransform.data[2][3] = 0.0f;

        rotationTransform.data[3][0] = 0.0f;
        rotationTransform.data[3][1] = 0.0f;
        rotationTransform.data[3][2] = 0.0f;
        rotationTransform.data[3][3] = 1.0f;

        result = matrixMultiply(result, rotationTransform);
    }

    return result;
}

void deleteShaderProgram(GLuint shaderProgram) noexcept
{
    assert(glGetError() == GL_NO_ERROR);
    glDeleteShader(shaderProgram);
}

GLuint getCompiledShaderProgram(const GLchar* headerSource, const GLchar* shaderSource, GLenum shaderType) noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    assert(headerSource && shaderSource);

    // Only these 2 shaders types are supported.
    assert(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

    GLuint shaderID = glCreateShader(shaderType);

    assert(shaderID);

    const GLchar* finalShaderSource[] = {
        headerSource,
        shaderSource,
    };

    // User null-terminated shader source.
    glShaderSource(shaderID, 2, finalShaderSource, nullptr);
    glCompileShader(shaderID);

    GLint shaderCompileStatus{};
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompileStatus);

    if (shaderCompileStatus == GL_FALSE)
    {
        GLint shaderLogSize{};
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &shaderLogSize);

        if (shaderLogSize > 0)
        {
            std::string shaderLog;
            shaderLog.reserve(static_cast<size_t>(shaderLogSize));

            glGetShaderInfoLog(shaderID, shaderLogSize, nullptr, (GLchar*)shaderLog.data());
            printf("%s", shaderLog.c_str());
        }

        if (shaderType == GL_VERTEX_SHADER)
        {
            haltWithMessage("Vertex shader compilation failed!\n");
        }
        else
        {
            haltWithMessage("Fragment shader compilation failed!\n");
        }
    }

    assert(glGetError() == GL_NO_ERROR);

    return shaderID;
}

GLuint getLinkedShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShaderID);
    glAttachShader(program, fragmentShaderID);

    glLinkProgram(program);

    GLint shaderLinkStatus{};
    glGetProgramiv(program, GL_LINK_STATUS, &shaderLinkStatus);

    if (shaderLinkStatus == GL_FALSE)
    {
        GLint shaderLogSize{};

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &shaderLogSize);

        if (shaderLogSize > 1)
        {
            std::string shaderLog;
            shaderLog.reserve(static_cast<size_t>(shaderLogSize));

            glGetProgramInfoLog(program, shaderLogSize, nullptr, (GLchar*)shaderLog.data());
            printf("%s", shaderLog.c_str());
        }

        glDeleteProgram(program);

        haltWithMessage("Program linking failed!\n");
    }

    assert(glGetError() == GL_NO_ERROR);

    return program;
}

}

void drawCubeShader(const ShaderContext& cubeShader, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    glValidateProgram(cubeShader.program);

    glViewport(0, 0, static_cast<GLsizei>(viewportWidth), static_cast<GLsizei>(viewportHeight));

    glClearColor(0.75, 0.75, 0.75, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspectRatio = static_cast<float>(viewportHeight) / static_cast<float>(viewportWidth);
    Matrix4x4 modelView = getIdentityMatrix();

    modelView = getAxisRotatedMatrix(modelView, (0.25f * static_cast<float>(frameCounter)), 1.0f, 0.0f, 0.0f);
    modelView = getAxisRotatedMatrix(modelView, (0.25f * static_cast<float>(frameCounter)), 0.0f, 1.0f, 0.0f);
    modelView = getAxisRotatedMatrix(modelView, (0.25f * static_cast<float>(frameCounter)), 0.0f, 0.0f, 1.0f);

    modelView = getScaledMatrix(modelView, 2.5f, 2.5f, 1.0f);

    modelView = getTranslatedMatrix(modelView, 0.0f, 0.0f, -7.0f);

    Matrix4x4 projection = getProjectionMatrix(-2.8f, 2.8f, -2.8f * aspectRatio, 2.8f * aspectRatio, 3.0f, 200.0f);

    Matrix4x4 modelViewProjection = getIdentityMatrix();
    modelViewProjection = matrixMultiply(modelView, projection);

    // Column-major order.
    glUniformMatrix4fv(cubeShader.modelViewProjectionMatrix, 1, GL_FALSE, &modelViewProjection.data[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, cubeShader.vbo);
    glBindTexture(GL_TEXTURE_2D, cubeShader.textureBinding);

    // Draw front face strip.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Draw back face strip.
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    // Draw right face strip.
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);

    // Draw left face strip.
    glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

    // Draw top face strip.
    glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);

    // Draw bottom face strip.
    glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    assert(glGetError() == GL_NO_ERROR);
}

ShaderContext createCubeShader() noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    ShaderContext cubeShader{};

    // Embedded shader header source string for both vertex and fragment shader.
    const GLchar* headerSource =
        R "shader(
#version 300 es
        precision mediump float;
    )shader";

    // Embedded vertex shader source string.
        const GLchar* vertexSource =
        R"shader(
    uniform mat4 modelViewProjectionMatrix;

    layout(location = 0) in vec4 vPosition;
    layout(location = 1) in vec2 vTex;

    out vec2 TexCoord;

    void main()
    {
        // Transform the vertex by the fused model-view-projection matrix to GL clip-space.

        gl_Position = modelViewProjectionMatrix * vPosition;

        // Passthrough UV-coordinates.

        TexCoord = vTex;
    }
    )shader";

    // Embedded fragment shader source string.
    const GLchar* fragmentSource =
        R"shader(
    in vec2 TexCoord;
    out vec4 FragColor;

    uniform sampler2D TexSampler;

    void main()
    {
        // Sample the 2D texture.

        FragColor = texture(TexSampler, TexCoord);
    }
    )shader";

    const GLuint vertexShaderProgram = getCompiledShaderProgram(headerSource, vertexSource, GL_VERTEX_SHADER);
    const GLuint fragmentShaderProgram = getCompiledShaderProgram(headerSource, fragmentSource, GL_FRAGMENT_SHADER);

    cubeShader.program = getLinkedShaderProgram(vertexShaderProgram, fragmentShaderProgram);

    deleteShaderProgram(vertexShaderProgram);
    deleteShaderProgram(fragmentShaderProgram);

    glUseProgram(cubeShader.program);

    cubeShader.modelViewProjectionMatrix = glGetUniformLocation(cubeShader.program, "modelViewProjectionMatrix");

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    cubeShader.positionsOffset = 0;
    cubeShader.UVOffset = sizeof(cubeVertices);

    glGenBuffers(1, &cubeShader.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cubeShader.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices) + sizeof(cubeUVs), 0, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, cubeShader.positionsOffset, sizeof(cubeVertices), cubeVertices);
    glBufferSubData(GL_ARRAY_BUFFER, cubeShader.UVOffset, sizeof(cubeUVs), cubeUVs);

    constexpr GLuint positionAttributeIndex = 0;
    glVertexAttribPointer(positionAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(static_cast<intptr_t>(cubeShader.positionsOffset)));
    glEnableVertexAttribArray(positionAttributeIndex);

    constexpr GLuint uvAttributeIndex = 1;
    glVertexAttribPointer(uvAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(static_cast<intptr_t>(cubeShader.UVOffset)));
    glEnableVertexAttribArray(uvAttributeIndex);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    assert(glGetError() == GL_NO_ERROR);

    return cubeShader;
}

void generateAndBindTexture(ShaderContext& shaderContext) noexcept
{
    assert(glGetError() == GL_NO_ERROR);
    assert(shaderContext.textureBinding == 0);

    glGenTextures(1, &shaderContext.textureBinding);
    glBindTexture(GL_TEXTURE_2D, shaderContext.textureBinding);

    assert(shaderContext.textureMemory);
    assert(shaderContext.textureBpp == 4);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shaderContext.textureWidth, shaderContext.textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, shaderContext.textureMemory);

    glBindTexture(GL_TEXTURE_2D, 0);

    assert(shaderContext.textureBinding != 0);
    assert(glGetError() == GL_NO_ERROR);
}

void setDefaultTextureParameters(const ShaderContext& shaderContext) noexcept
{
    assert(glGetError() == GL_NO_ERROR);
    assert(shaderContext.textureBinding != 0);

    glBindTexture(GL_TEXTURE_2D, shaderContext.textureBinding);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    assert(shaderContext.textureBinding != 0);
    assert(glGetError() == GL_NO_ERROR);
}
