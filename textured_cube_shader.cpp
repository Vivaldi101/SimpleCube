#include <textured_cube_shader.hpp>

#include <cmath>
#include <string>
#include <cassert>

#define Invariant(cond) do { if (!(cond)) __debugbreak(); } while (0)

#ifdef _DEBUG
#define Implies(a, b) (!(a) || (b))
#define Equals(a, b) Implies((a), (b)) && Implies((b), (a))
#else
#define Implies(a, b) (!(a) || (b))
#endif

void print(const char* format, ...);
namespace
{
    void kzHaltWithMessage(const char* msg)
    {
        print(msg);
        Invariant(false);
    }

// Checkerboard pattern.
constexpr unsigned int defaultTestTexture[] = {
    0xff44aacc, 0xffffffff,
	0xffffffff, 0xff44aacc,
};

constexpr float PI{ 3.1415926535897932384626433832795f };

constexpr GLfloat cubeVertices[] = 
{
    // 3D coordinates extended to 4D homogeneous clip-space in vertex shader.

    +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f,

    -1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f,

    -1.0f, +1.0f, +1.0f, -1.0f, -1.0f, +1.0f,

    +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f,

    +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f,

    +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, -1.0f,

    -1.0f, +1.0f, +1.0f, -1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f,

    -1.0f, +1.0f, -1.0f, -1.0f, -1.0f, -1.0f,

    -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f,

    +1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f,

    +1.0f, +1.0f, -1.0f, -1.0f, +1.0f, -1.0f,

    +1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f,

    -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, +1.0f,

    -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, +1.0f, -1.0f,

    +1.0f, -1.0f, -1.0f, +1.0f, -1.0f, -1.0f,

    -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f,
};

constexpr GLfloat cubeStripVertices[] = 
{
    // 3D coordinates extended to 4D homogeneous clip-space in vertex shader.

    // Front face.

    +1.0f, -1.0f, +1.0f,

    +1.0f, +1.0f, +1.0f,

    -1.0f, -1.0f, +1.0f,

    -1.0f, +1.0f, +1.0f,

    // Back face.

    -1.0f, -1.0f, -1.0f,

    -1.0f, +1.0f, -1.0f,

    +1.0f, -1.0f, -1.0f,

    +1.0f, +1.0f, -1.0f,

    // Right face.

    +1.0f, -1.0f, -1.0f,

    +1.0f, +1.0f, -1.0f,

    +1.0f, -1.0f, +1.0f,

    +1.0f, +1.0f, +1.0f,

    // Left face.

    -1.0f, -1.0f, +1.0f,

    -1.0f, +1.0f, +1.0f,

    -1.0f, -1.0f, -1.0f,

    -1.0f, +1.0f, -1.0f,

    // Top face.

    +1.0f, +1.0f, +1.0f,

    +1.0f, +1.0f, -1.0f,

    -1.0f, +1.0f, +1.0f,

    -1.0f, +1.0f, -1.0f,

    // Bottom face.

    +1.0f, -1.0f, -1.0f,

    +1.0f, -1.0f, +1.0f,

    -1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, +1.0f,
};

struct CubeFaceUVCoordinates
{
    const float bottomRight[2]{ 1.0f, 0.0f };
    const float topRight[2]{ 1.0f, 1.0f };
    const float bottomleft[2]{ 0.0f, 0.0f };
    const float topLeft[2]{ 0.0f, 1.0f };
};

constexpr CubeFaceUVCoordinates cubeUVs[6];

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
	assert(glIsShader(shaderProgram) && "Not valid vertex shader");

    glDeleteShader(shaderProgram);

    assert(glGetError() == GL_NO_ERROR);
}

GLuint getCompiledShaderProgram(const GLchar* headerSource, const GLchar* shaderSource, GLenum shaderType) noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    assert(headerSource && shaderSource);

    // Only these 2 shaders types are supported.
    assert(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateShader(shaderType);

    assert(shaderProgram);

    const GLchar* finalShaderSource[] = {
        headerSource,
        shaderSource,
    };

    // User null-terminated shader source.
    glShaderSource(shaderProgram, 2, finalShaderSource, nullptr);
    glCompileShader(shaderProgram);

    GLint shaderCompileStatus{};
    glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &shaderCompileStatus);

    if (shaderCompileStatus == GL_FALSE)
    {
        GLint shaderLogSize{};
        glGetShaderiv(shaderProgram, GL_INFO_LOG_LENGTH, &shaderLogSize);

        if (shaderLogSize > 0)
        {
            std::string shaderLog;
            shaderLog.reserve(static_cast<size_t>(shaderLogSize));

            glGetShaderInfoLog(shaderProgram, shaderLogSize, nullptr, (GLchar*)shaderLog.data());
            printf("%s", shaderLog.c_str());
        }

        if (shaderType == GL_VERTEX_SHADER)
        {
            kzHaltWithMessage("Vertex shader compilation failed!\n");
        }
        else
        {
            kzHaltWithMessage("Fragment shader compilation failed!\n");
        }
    }

    assert(glGetError() == GL_NO_ERROR);

    Invariant(shaderProgram > 0);

    return shaderProgram;
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

        kzHaltWithMessage("Program linking failed!\n");
    }

    assert(glGetError() == GL_NO_ERROR);

    Invariant(glIsProgram(program));

    return program;
}

void drawTriangleStrips(const ShaderContext& shaderContext, unsigned int stripCount) noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    for (unsigned int i = 0; i < stripCount; ++i)
    {
        glDrawArrays(GL_TRIANGLE_STRIP, i*4, 4);
    }

    assert(glGetError() == GL_NO_ERROR);
}

}

void setupCubeShaderView(ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    const float aspectRatio = static_cast<float>(viewportHeight) / static_cast<float>(viewportWidth);
    Matrix4x4 modelView = getIdentityMatrix();

    // TODO: Pass in the MVP matrix.
    modelView = getAxisRotatedMatrix(modelView, (0.0725f * static_cast<float>(frameCounter)), 1.0f, 0.0f, 0.0f);
    modelView = getAxisRotatedMatrix(modelView, (0.0725f * static_cast<float>(frameCounter)), 0.0f, 1.0f, 0.0f);
    modelView = getAxisRotatedMatrix(modelView, (0.0725f * static_cast<float>(frameCounter)), 0.0f, 0.0f, 1.0f);

    modelView = getScaledMatrix(modelView, 2.5f, 2.5f, 1.0f);

    modelView = getTranslatedMatrix(modelView, 0.0f, 0.0f, -7.0f);

    Matrix4x4 projection = getProjectionMatrix(-2.8f, 2.8f, -2.8f * aspectRatio, 2.8f * aspectRatio, 3.0f, 200.0f);

    Matrix4x4 modelViewProjection = getIdentityMatrix();
    modelViewProjection = matrixMultiply(modelView, projection);

    // Column-major order.
    glUniformMatrix4fv(shaderContext.modelViewProjectionMatrixUniform, 1, GL_FALSE, &modelViewProjection.data[0][0]);

    shaderContext.modelViewProjection = modelViewProjection;

    assert(glGetError() == GL_NO_ERROR);
}

void drawCubeShaderToTexture(ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept
{
	assert(glIsProgram(shaderContext.rttProgram));

	glUseProgram(shaderContext.rttProgram);

    glUniform1ui(shaderContext.objectIDUniform, 1);
	glUniform1ui(shaderContext.drawIDUniform, 1);

	// Bind cube vertex attribute arrays.
	glBindVertexArray(shaderContext.cubePickingVAO);

    shaderContext.modelViewProjectionMatrixUniform = glGetUniformLocation(shaderContext.rttProgram, "modelViewProjectionMatrix");
    setupCubeShaderView(shaderContext, viewportWidth, viewportHeight, frameCounter);

    // Bind the texture to map onto the cube.
    glBindTexture(GL_TEXTURE_2D, shaderContext.textureBinding);

    glViewport(0, 0, static_cast<GLsizei>(viewportWidth), static_cast<GLsizei>(viewportHeight));

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw the textured cube.
    glDrawArrays(GL_TRIANGLES, 0, 3 * 12);

    // Detach vertex buffer binding.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Detach texture binding.
    glBindTexture(GL_TEXTURE_2D, 0);

	assert(glIsProgram(shaderContext.cubeProgram));

    // Detach current shader programs.
    glUseProgram(0);
}

void drawTexturedCubeShaderToOutput(ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int frameCounter) noexcept
{
	assert(glIsProgram(shaderContext.cubeProgram));

	glUseProgram(shaderContext.cubeProgram);

	// Bind cube vertex array attributes.
	glBindVertexArray(shaderContext.cubeVAO);

    shaderContext.modelViewProjectionMatrixUniform = glGetUniformLocation(shaderContext.cubeProgram, "modelViewProjectionMatrix");
    setupCubeShaderView(shaderContext, viewportWidth, viewportHeight, frameCounter);

    // Bind the texture to map onto the cube.
    glBindTexture(GL_TEXTURE_2D, shaderContext.textureBinding);

    glViewport(0, 0, static_cast<GLsizei>(viewportWidth), static_cast<GLsizei>(viewportHeight));

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw all the faces of the cube.
    drawTriangleStrips(shaderContext, 6);

    // Detach vertex buffer binding.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Detach texture binding.
    glBindTexture(GL_TEXTURE_2D, 0);

	assert(glIsProgram(shaderContext.cubeProgram));

    // Detach current shader programs.
    glUseProgram(0);
}

void drawCubeShaderElapsed(const ShaderContext& shaderContext, unsigned int viewportWidth, unsigned int viewportHeight, unsigned int usElapsed) noexcept
{
    assert(glGetError() == GL_NO_ERROR);

    glViewport(0, 0, static_cast<GLsizei>(viewportWidth), static_cast<GLsizei>(viewportHeight));

    glClearColor(0.1f, 0.1f, 0.1f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspectRatio = static_cast<float>(viewportHeight) / static_cast<float>(viewportWidth);
    Matrix4x4 modelView = getIdentityMatrix();

    modelView = getAxisRotatedMatrix(modelView, (0.25f * static_cast<float>(usElapsed) / 10000), 1.0f, 0.0f, 0.0f);
    modelView = getAxisRotatedMatrix(modelView, (0.25f * static_cast<float>(usElapsed) / 10000), 0.0f, 1.0f, 0.0f);
    modelView = getAxisRotatedMatrix(modelView, (0.25f * static_cast<float>(usElapsed) / 10000), 0.0f, 0.0f, 1.0f);

    modelView = getScaledMatrix(modelView, 2.5f, 2.5f, 1.0f);

    modelView = getTranslatedMatrix(modelView, 0.0f, 0.0f, -7.0f);

    Matrix4x4 projection = getProjectionMatrix(-2.8f, 2.8f, -2.8f * aspectRatio, 2.8f * aspectRatio, 3.0f, 200.0f);

    Matrix4x4 modelViewProjection = getIdentityMatrix();
    modelViewProjection = matrixMultiply(modelView, projection);

    glUniform1f(shaderContext.uvRepeatCountUniform, shaderContext.uvRepeatCount);

    // Column-major order.
    glUniformMatrix4fv(shaderContext.modelViewProjectionMatrixUniform, 1, GL_FALSE, &modelViewProjection.data[0][0]);

    // Bind the vertex buffers to use for the cube.
    glBindBuffer(GL_ARRAY_BUFFER, shaderContext.cubeVBO);

    // Bind the texture to map onto the cube.
    glBindTexture(GL_TEXTURE_2D, shaderContext.textureBinding);

    drawTriangleStrips(shaderContext, 6);

    // Detach bindings.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    assert(glGetError() == GL_NO_ERROR);
}

void generateAndBindTexture(ShaderContext& shaderContext) noexcept
{
    assert(glGetError() == GL_NO_ERROR);
    assert(shaderContext.textureWidth > 0);
    assert(shaderContext.textureHeight > 0);
    assert(shaderContext.uvRepeatCount > 0);

    assert(shaderContext.textureBpp == 4);
    assert(shaderContext.textureMemory);

    glGenTextures(1, &shaderContext.textureBinding);
    glBindTexture(GL_TEXTURE_2D, shaderContext.textureBinding);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shaderContext.textureWidth, shaderContext.textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, shaderContext.textureMemory);

    glBindTexture(GL_TEXTURE_2D, 0);

    assert(shaderContext.textureBinding != 0);
    assert(glGetError() == GL_NO_ERROR);
}

void generateAndBindDefaultTexture(ShaderContext& shaderContext) noexcept
{
    static_assert(sizeof(defaultTestTexture) / sizeof(*defaultTestTexture) == 4, "Default texture must be 2x2 size");

    shaderContext.textureWidth = 2;
    shaderContext.textureHeight = 2;
    shaderContext.textureBpp = sizeof(defaultTestTexture[0]);
    shaderContext.textureMemory = defaultTestTexture;

    // Repeat the texture pattern.
    shaderContext.uvRepeatCount = 2.5f;
	assert(glIsProgram(shaderContext.cubeProgram));
    glUniform1f(shaderContext.uvRepeatCountUniform, shaderContext.uvRepeatCount);

    generateAndBindTexture(shaderContext);
}

void setDefaultGLTextureParameters(const ShaderContext& shaderContext) noexcept
{
    assert(glGetError() == GL_NO_ERROR);
    assert(shaderContext.textureBinding != 0);

    glBindTexture(GL_TEXTURE_2D, shaderContext.textureBinding);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    assert(shaderContext.textureBinding != 0);
    assert(glGetError() == GL_NO_ERROR);
}

ShaderContext createCubeShader() noexcept
{
		// Load OpenGL functions.
#define X(type, name) name = (type)wglGetProcAddress(#name); assert(name);
		GL_FUNCTIONS(X)
#undef X

    assert(glGetError() == GL_NO_ERROR);

    ShaderContext cubeShader = {};

    // TODO: Separate shader loading.

    // Embedded shader header source string for both vertex and fragment shader.
    const GLchar* headerSource =
    R"kz_shader(
        #version 450 core 
    )kz_shader";

    // Embedded vertex shader source string.
        const GLchar* cubeVertexShaderSource =
        R"kz_shader(
    uniform mat4 modelViewProjectionMatrix;
    uniform float uvRepeatCount;

    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec2 uv;

    out vec2 uvRepeat;

    void main()
    {
        // Transform the vertex by the fused model-view-projection matrix to GL clip-space.

        gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0f);

        // Scale UVs by the repeat count for the texture pattern.

        uvRepeat = uv * uvRepeatCount;
    }
    )kz_shader";

    // Embedded fragment shader source string.
		const GLchar* cubeFragmentShaderSource =
		R"kz_shader(
            uniform sampler2D TexSampler;

            layout(location = 0) 
            out vec4 fragmentColor;

            layout(location = 1) 
            in vec2 uvRepeat;

            void main()
            {
                // Sample the 2D texture.

                fragmentColor = texture(TexSampler, uvRepeat);
            }
        )kz_shader";

		// Embedded fragment shader source string.
		const GLchar* cubeRTTFragmentShaderSource =
		R"kz_shader(
				layout (location = 0)
				out uvec3 fragment;

                layout(location = 1)

				uniform uint objectID;
				uniform uint drawID;

				void main()
				{
					 fragment = uvec3(objectID, drawID, gl_PrimitiveID);
				}
        )kz_shader";

    // Textured cube shader compilation/linking.
	{
		const GLuint cubeVertexShaderProgram = getCompiledShaderProgram(headerSource, cubeVertexShaderSource, GL_VERTEX_SHADER);
		const GLuint cubeFragmentShaderProgram = getCompiledShaderProgram(headerSource, cubeFragmentShaderSource, GL_FRAGMENT_SHADER);
		const GLuint cubeRTTFragmentShaderProgram = getCompiledShaderProgram(headerSource, cubeRTTFragmentShaderSource, GL_FRAGMENT_SHADER);

		cubeShader.cubeProgram = getLinkedShaderProgram(cubeVertexShaderProgram, cubeFragmentShaderProgram);
		cubeShader.rttProgram = getLinkedShaderProgram(cubeVertexShaderProgram, cubeRTTFragmentShaderProgram);

		deleteShaderProgram(cubeVertexShaderProgram);
		deleteShaderProgram(cubeFragmentShaderProgram);
		deleteShaderProgram(cubeRTTFragmentShaderProgram);
	}

    cubeShader.uvRepeatCountUniform = glGetUniformLocation(cubeShader.cubeProgram, "uvRepeatCount");

    cubeShader.objectIDUniform = glGetUniformLocation(cubeShader.rttProgram, "objectID");
    cubeShader.drawIDUniform = glGetUniformLocation(cubeShader.rttProgram, "drawID");

    //assert(cubeShader.modelViewProjectionMatrixUniform >= 0);
    assert(cubeShader.uvRepeatCountUniform >= 0);

    assert(cubeShader.objectIDUniform >= 0);
    assert(cubeShader.drawIDUniform >= 0);

    cubeShader.positionsOffset = 0;
    cubeShader.UVOffset = sizeof(cubeStripVertices);

    // Cube shader VAO/VBO setup.
    {

		glGenVertexArrays(1, &cubeShader.cubeVAO);
		glBindVertexArray(cubeShader.cubeVAO);

		assert(glIsVertexArray(cubeShader.cubeVAO));

        glGenBuffers(1, &cubeShader.cubeVBO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeShader.cubeVBO);

	    assert(glIsBuffer(cubeShader.cubeVBO));

        // Allocate buffer.
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeStripVertices) + sizeof(cubeUVs), nullptr, GL_STATIC_DRAW);

        // Copy the vertices and uvs into the vbo.
        glBufferSubData(GL_ARRAY_BUFFER, cubeShader.positionsOffset, sizeof(cubeStripVertices), cubeStripVertices);
        glBufferSubData(GL_ARRAY_BUFFER, cubeShader.UVOffset, sizeof(cubeUVs), cubeUVs);

        constexpr GLuint positionAttributeIndex = 0;

        // Setup vertex attribute format for vertex positions and where to fetch it.
        glVertexAttribPointer(positionAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(static_cast<intptr_t>(cubeShader.positionsOffset)));

        // Enable the attribute.
        glEnableVertexAttribArray(positionAttributeIndex);

        constexpr GLuint uvAttributeIndex = 1;

        // Setup vertex attribute format for UVs and where to fetch it.
        glVertexAttribPointer(uvAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(static_cast<intptr_t>(cubeShader.UVOffset)));

        // Enable the attribute.
        glEnableVertexAttribArray(uvAttributeIndex);

        // Detach vertex buffer and array attributes.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Cube picking shader VBO setup.
    {
		glGenVertexArrays(1, &cubeShader.cubePickingVAO);
		glBindVertexArray(cubeShader.cubePickingVAO);

		assert(glIsVertexArray(cubeShader.cubePickingVAO));

        glGenBuffers(1, &cubeShader.cubePickingVBO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeShader.cubePickingVBO);

	    assert(glIsBuffer(cubeShader.cubePickingVBO));

        // Allocate buffer.
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), nullptr, GL_STATIC_DRAW);

        // Copy the vertices into the VBO.
        glBufferSubData(GL_ARRAY_BUFFER, cubeShader.positionsOffset, sizeof(cubeVertices), cubeVertices);

        constexpr GLuint positionAttributeIndex = 0;

        // Setup vertex attribute format for vertex positions and where to fetch it.
        glVertexAttribPointer(positionAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(static_cast<intptr_t>(cubeShader.positionsOffset)));

        // Enable the attribute.
        glEnableVertexAttribArray(positionAttributeIndex);

        // Detach vertex buffer and array attributes.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glUseProgram(cubeShader.cubeProgram);

    // Use the default texture for the cube.
    generateAndBindDefaultTexture(cubeShader);
    setDefaultGLTextureParameters(cubeShader);

    // Detach current shader programs.
    glUseProgram(0);

    assert(glGetError() == GL_NO_ERROR);

    return cubeShader;
}
