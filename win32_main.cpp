// example how to set up OpenGL core context on Windows
// and use basic functionality of OpenGL 4.5 version

// important extension functionality used here:
// (4.3) KHR_debug:                     https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt
// (4.5) ARB_direct_state_access:       https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_direct_state_access.txt
// (4.1) ARB_separate_shader_objects:   https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_separate_shader_objects.txt
// (4.2) ARB_shading_language_420pack:  https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_shading_language_420pack.txt
// (4.3) ARB_explicit_uniform_location: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_explicit_uniform_location.txt

#include <textured_cube_shader.hpp>

#define EQ(n, p) [&]() -> bool {for(size_t i__ = 0u; i__ < (n); ++i__) { if ((p)) { return true; } } return false; }()
#define UQ(n, p) [&]() -> bool {for(size_t i__ = 0u; i__ < (n); ++i__) { if (!(p)) { return false; } } return true; }()
#define CQ(n, p) [&]() -> unsigned int {unsigned int counter = 0; for(size_t i__ = 0u; i__ < (n); ++i__) { if ((p)) { ++counter; } } return counter; }()


#define _USE_MATH_DEFINES
#include <math.h>
#include <stddef.h>
#include <cassert>
#include <cstdio>
#include <cmath>
#include <array>
#include <string>
#include <span>
#include <string_view>
#include <algorithm>
#include <vector>
#include <optional>

#include <intrin.h>
#ifdef _DEBUG
#define Invariant(cond) do { if (!(cond)) __debugbreak(); } while (0)
#define Implies(a, b) (!(a) || (b))
#define Equals(a, b) Implies((a), (b)) && Implies((b), (a))
#else
#define Invariant(cond)
#define Implies(a, b) (!(a) || (b))
#endif

#define kzVaArgs(format, buffer)                                                                 \
    {                                                                                            \
        static_assert(!(buffer).empty(), "Format buffer must not be empty");                     \
        static_assert(sizeof(buffer) == (buffer).size(), "Format buffer must be a byte buffer"); \
        va_list argList;                                                                         \
        va_start(argList, format);                                                               \
        vsnprintf((buffer).data(), (buffer).size(), format, argList);                            \
        va_end(argList);                                                                         \
    }

#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "opengl32.lib")

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

#ifndef NDEBUG
static void APIENTRY DebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* user)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");

    if (severity != GL_DEBUG_SEVERITY_HIGH && severity != GL_DEBUG_SEVERITY_MEDIUM)
		return;

	if (severity == GL_DEBUG_SEVERITY_MEDIUM)
		return;

	if (IsDebuggerPresent())
	{
		Invariant(!"OpenGL error - check the callstack in debugger");
	}

	FatalError("OpenGL API usage error! Use debugger to examine call stack!");
}
#endif

static bool globalIsMouseButtonDown;

static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

		// Mouse down events.
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	{
		globalIsMouseButtonDown ^= 1;
		break;
	}

	// Mouse up events.
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		globalIsMouseButtonDown ^= 1;
		break;
	}

    }
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

struct Position
{
	int x, y;
};

struct TextureContext
{
	GLuint textureBinding;
	GLuint rttBinding;

	int textureWidth;
    int textureHeight;
    int textureBpp;
	const void* textureMemory;
};

struct ProgramPipeline
{
    GLuint pipeline;
    GLuint vertexShader;
    GLuint fragmentShader;

    // TODO: Split these.
	GLint vertexTransformUniform;
	GLint mousePositionUniform;
};

struct Vertex
{
	float position[2];
	//float uv[2];
	//float color[3];
};

void print(const char* format, ...)
{
    static std::array<char, 1024> vaArgsBuffer;

    kzVaArgs(format, vaArgsBuffer)

    // Write to debug window.
    OutputDebugStringA(vaArgsBuffer.data());

    // Write to console.
    static HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdOut != NULL && stdOut != INVALID_HANDLE_VALUE)
    {
        const DWORD charCount = static_cast<DWORD>(strlen(vaArgsBuffer.data()));
        DWORD written = 0;
        WriteConsoleA(stdOut, vaArgsBuffer.data(), charCount, &written, NULL);
    }
}

static void drawText(const char* text, ...)
{
    // draw to window ...
}

static void drawQuad()
{
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

// TODO:
// Re-use this
static void drawTriangle(int index)
{
	glDrawArrays(GL_TRIANGLES, index, 3);
}

static ProgramPipeline createProgramPipeline(const char* vShader, const char* fShader)
{
	assert(glGetError() == GL_NO_ERROR);

    ProgramPipeline result = {}; 

    result.vertexShader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vShader);
    result.fragmentShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fShader);

    GLint linked = {};

	glGetProgramiv(result.vertexShader, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		char message[1024];
		glGetProgramInfoLog(result.vertexShader, sizeof(message), NULL, message);
		OutputDebugStringA(message);

		Invariant(!"Failed to create vertex shader!");
	}

	glGetProgramiv(result.fragmentShader, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		char message[1024];
		glGetProgramInfoLog(result.fragmentShader, sizeof(message), NULL, message);
		OutputDebugStringA(message);

		Invariant(!"Failed to create fragment shader!");
	}

	glGenProgramPipelines(1, &result.pipeline);
	glUseProgramStages(result.pipeline, GL_VERTEX_SHADER_BIT, result.vertexShader);
	glUseProgramStages(result.pipeline, GL_FRAGMENT_SHADER_BIT, result.fragmentShader);

    if (!glIsProgramPipeline(result.pipeline))
    {
		char message[1024];
		glGetProgramInfoLog(result.vertexShader, sizeof(message), NULL, message);
		OutputDebugStringA(message);

		Invariant(!"Failed to create program pipeline!");
    }

	assert(glGetError() == GL_NO_ERROR);

    return result;
}

static void bindProgramPipeline(ProgramPipeline pp)
{
    glBindProgramPipeline(pp.pipeline);
}

Position getCursorWindowPosition(HWND window, int windowWidth, int windowHeight)
{
	Position result = {};

	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);

	ScreenToClient(window, &cursorPoint);

	result.x = cursorPoint.x;
	result.y = cursorPoint.y;

	return result;
}

// compares src string with dstlen characters from dst, returns 1 if they are equal, 0 if not
static int StringsAreEqual(const char* src, const char* dst, size_t dstlen)
{
	while (*src && dstlen-- && *dst)
    {
        if (*src++ != *dst++)
        {
            return 0;
        }
    }

    return (dstlen && *src == *dst) || (!dstlen && *src == 0);
}

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

static void GetWglFunctions(void)
{
    // to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        100, 100, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    Invariant(dummy && "Failed to create dummy window");

    HDC dc = GetDC(dummy);
    Invariant(dc && "Failed to get device context for dummy window");

	PIXELFORMATDESCRIPTOR desc = {};
	desc.nSize = sizeof(desc);
	desc.nVersion = 1;
	desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	desc.iPixelType = PFD_TYPE_RGBA;
	desc.cColorBits = 24;

    int format = ChoosePixelFormat(dc, &desc);
    if (!format)
    {
        FatalError("Cannot choose OpenGL pixel format for dummy window!");
    }

    int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
    Invariant(ok && "Failed to describe OpenGL pixel format");

    // reason to create dummy window is that SetPixelFormat can be called only once for the window
    if (!SetPixelFormat(dc, format, &desc))
    {
        FatalError("Cannot set OpenGL pixel format for dummy window!");
    }

    HGLRC rc = wglCreateContext(dc);
    Invariant(rc && "Failed to create OpenGL context for dummy window");

    ok = wglMakeCurrent(dc, rc);
    Invariant(ok && "Failed to make current OpenGL context for dummy window");

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
        (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    if (!wglGetExtensionsStringARB)
    {
        FatalError("OpenGL does not support WGL_ARB_extensions_string extension!");
    }

    const char* ext = wglGetExtensionsStringARB(dc);

    const std::string_view extension{ wglGetExtensionsStringARB(dc) };

    std::vector<std::string> extensionNames{};

    Invariant(ext && "Failed to get OpenGL WGL extension string");

    const char* start = ext;
    for (;;)
    {
        while (*ext != 0 && *ext != ' ')
        {
            ext++;
        }

        if (*ext == 0)
        {
            break;
        }

        size_t length = ext - start;
        if (StringsAreEqual("WGL_ARB_pixel_format", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
            wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        }
        else if (StringsAreEqual("WGL_ARB_create_context", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
            wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        }
        else if (StringsAreEqual("WGL_EXT_swap_control", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
            wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        }

        ext++;
        start = ext;
    }

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT)
    {
        FatalError("OpenGL does not support required WGL extensions for modern context!");
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);
}

struct PixelBufferData
{
	unsigned int objectID;
	unsigned int drawID;
	unsigned int primitiveID;
};

void drawCubeShaderToTexture(ShaderContext& context, int width, int height, unsigned int counter, GLuint frameBuffer) 
{
	assert(glGetError() == GL_NO_ERROR);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// Draw cube into texture.
	drawCubeShaderToTexture(context, width, height, counter);

	// Restore default frame buffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	assert(glGetError() == GL_NO_ERROR);
}

PixelBufferData readFromTextureCube(int x, int y, int width, int height, GLuint frameBuffer)
{
	PixelBufferData result = {};

	// Read from frame buffer 
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);

	// Read from color texture
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glFlush();
	glFinish();

	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	glReadPixels(x, height - y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &result);

	// Restore default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return result;
}

static void drawPrimitive(int primitiveID)
{
	drawTriangle(primitiveID*3);
}

static Position globalOldCursor;

static bool isCursorPosChanged(Position cursor)
{
	if (cursor.x == globalOldCursor.x && cursor.y == globalOldCursor.y)
	{
		Invariant(cursor.x == globalOldCursor.x && cursor.y == globalOldCursor.y);

		return false;
	}

	Invariant(cursor.x != globalOldCursor.x || cursor.y != globalOldCursor.y);

	globalOldCursor = cursor;

	return true;
}

static unsigned int globalOldID;

static bool isPrimitiveIDChanged(unsigned int ID)
{
	if (globalOldID == ID)
	{
		Invariant(globalOldID == ID);

		return false;
	}

	Invariant(globalOldID != ID);

	globalOldID = ID;

	return true;
}

int narrowMult(int i)
{
	assert(i >= INT_MIN / 7 && i <= INT_MAX / 7);
	// wp(i * 7 <= INT_MAX)

	// (i <= INT_MAX / 7*7)
	// => (i <= INT_MAX / 7)

	// (i <= INT_MAX / 7) &&
	// wp(i := i / 7, i <= INT_MAX)

	// wp(i := (INT_MAX / 7) / 7, i <= INT_MAX)

	// wp(((INT_MAX / 7) / 7) <= INT_MAX)

	// wp(INT_MAX / (7*7) <= INT_MAX)

	// T

	// wp(i * 7 <= INT_MAX) && T

	// wp(i * 7 <= INT_MAX)
	// --------------------

	// wp(i * 7 >= INT_MIN)
	// => wp(i >= INT_MIN / 7)

	// (i >= INT_MIN / 7) &&
	// wp(i := i / 7, i >= INT_MIN)

	// wp(i := INT_MIN / 7 / 7, i >= INT_MIN)

	// wp(i := INT_MIN / (7*7), i >= INT_MIN)

	// wp(INT_MIN / (7*7) >= INT_MIN)
	// T

	return (i * 7) / 7;
}

void fillReverse(size_t* arr, size_t size)
{
	assert(size >= 0);

	size_t i = size;

	// Pre-condition.
	// Invariant range: 0 < i <= size
	assert(UQ(0, arr[i__] <= arr[i__+1]));

	//while (i != 0) 
	for (size_t i = size; i != 0; i--) 
	{
		// Invariant range: 0 < i <= size
		assert(UQ(i-1, arr[i__] <= arr[i__+1]));

		assert(i - 1 >= 0);
		arr[i - 1] = i;

		//i--;

		// Invariant range: 0 < i <= size
		assert(UQ(i, arr[i__] <= arr[i__+1]));

		// wp(i := i - 1, P)
		// (UQ(i-1, arr[i__] <= arr[i__+1])) && i != 0 => i-1 >= 0
	}

	// Post-condition.
	// Invariant range: 0 < i <= size
	assert(UQ(size, arr[i__] <= arr[i__+1]));
}

namespace
{
template <typename T>
void foo(T* t)
{
	t->i = 42;
}
}

struct S
{
	int i;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
	{
		S s = {};
		foo(&s);
	}
#if 0
	for (int i = INT_MIN; i < INT_MAX; ++i)
	{
		if (i >= INT_MIN / 7 && i <= INT_MAX / 7)
		{
			assert(i >= INT_MIN / 7 && i <= INT_MAX / 7);
			const int result = narrowMult(i);

			if (result != i)
			{
				FatalError("Fatal!");
			}
		}
	}
#endif

	AttachConsole(ATTACH_PARENT_PROCESS);

	// get WGL functions to be able to create modern GL context
	GetWglFunctions();

	// register window class to have custom WindowProc callback
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"opengl_window_class";
	ATOM atom = RegisterClassExW(&wc);
	Invariant(atom && "Failed to register window class");

	// window properties - width, height and style
	int width = CW_USEDEFAULT;
	int height = CW_USEDEFAULT;
	DWORD exstyle = WS_EX_APPWINDOW;
	DWORD style = WS_POPUPWINDOW;

	// uncomment in case you want fixed size window
	style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX & ~WS_BORDER;
	RECT rect = { 0, 0, 800, 600 };
	AdjustWindowRectEx(&rect, style, FALSE, exstyle);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	// create window
	HWND window = CreateWindowExW(
		exstyle, wc.lpszClassName, L"OpenGL Window", style,
		1920/2 - width/2, 1080/2 - height/2, width, height,
		NULL, NULL, wc.hInstance, NULL);
	Invariant(window && "Failed to create window");

	RECT clientRectangle = {};
	GetClientRect(window, &clientRectangle);

	width = clientRectangle.right - clientRectangle.left;
	height = clientRectangle.bottom - clientRectangle.top;

	Invariant(width > 0 && width <= (rect.right - rect.left));
	Invariant(height > 0 && height <= (rect.bottom - rect.top));

	const float aspectRatio = (float)height / width;

	HDC dc = GetDC(window);
	Invariant(dc && "Failed to window device context");

	// set pixel format for OpenGL context
	{
		int attrib[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
			WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB,     24,
			WGL_DEPTH_BITS_ARB,     24,
			WGL_STENCIL_BITS_ARB,   8,

			// uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
			//WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

			// uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
			// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
			//WGL_SAMPLE_BUFFERS_ARB, 1,
			//WGL_SAMPLES_ARB,        4, // 4x MSAA

			0,
		};

		int format;
		UINT formats;
		if (!wglChoosePixelFormatARB(dc, attrib, NULL, 1, &format, &formats) || formats == 0)
		{
			FatalError("OpenGL does not support required pixel format!");
		}

		PIXELFORMATDESCRIPTOR desc = {};
		desc.nSize = sizeof(desc);
		int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
		Invariant(ok && "Failed to describe OpenGL pixel format");

		if (!SetPixelFormat(dc, format, &desc))
		{
			FatalError("Cannot set OpenGL selected pixel format!");
		}
	}

	// create modern OpenGL context
	{
		int attrib[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 5,
			WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifndef NDEBUG
			// ask for debug context for non "Release" builds
			// this is so we can enable debug callback
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
			0,
		};

		HGLRC rc = wglCreateContextAttribsARB(dc, NULL, attrib);
		if (!rc)
		{
			FatalError("Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");
		}

		BOOL ok = wglMakeCurrent(dc, rc);
		Invariant(ok && "Failed to make current OpenGL context");

		// load OpenGL functions
#define X(type, name) name = (type)wglGetProcAddress(#name); Invariant(name);
		GL_FUNCTIONS(X)
#undef X

#ifndef NDEBUG
			// enable debug callback
			glDebugMessageCallback(&DebugCallback, NULL);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
	}

	ShaderContext cubeShader = createCubeShader();

	GLuint quadVAO = 0;
	{
		// TODO: wrap the quad vao

		glGenVertexArrays(1, &quadVAO);

		glBindVertexArray(quadVAO);

		Invariant(glIsVertexArray(quadVAO));

		// bind and enable vertex buffers to the bind quad VAO for axis-pipeline
		GLuint quadVBO = 0;
		{
			const Vertex verts[] =
			{
				// 2D coordinates extended to 4D homogeneous clip-space in vertex shader.
				// upper-left triangle
				{ -1.00f, -1.00f },
				{ +1.00f, +1.00f },
				{ -1.00f, +1.00f },

				// lower-right triangle
				{ -1.00f, -1.00f },
				{ +1.00f, -1.00f },
				{ +1.00f, +1.00f },
			};

			glGenBuffers(1, &quadVBO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferStorage(GL_ARRAY_BUFFER, sizeof(verts), verts, 0);
		}

		GLint vboIndex = 0;
		glBindVertexBuffer(vboIndex, quadVBO, 0, sizeof(struct Vertex));

		GLint vertexIndex = 0;
		glVertexAttribFormat(vertexIndex, 2, GL_FLOAT, GL_FALSE, offsetof(struct Vertex, position));
		glVertexAttribBinding(vertexIndex, vboIndex);
		glEnableVertexAttribArray(vertexIndex);
	}

	// render to texture
	GLuint rttFramebuffer = 0;
	{
		GLuint rttTexture = 0;
		glGenTextures(1, &rttTexture);
		glBindTexture(GL_TEXTURE_2D, rttTexture);

		// make the texture the same size as the viewport
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, width, height, 0, GL_RGB_INTEGER, GL_UNSIGNED_INT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenFramebuffers(1, &rttFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, rttFramebuffer);

		// attach colour texture to fb
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rttTexture, 0);

		glBindTexture(GL_TEXTURE_2D, 0);

		// redirect fragment shader output 0 used to the texture that we just bound
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			FatalError("Incomplete framebuffer status!");
		}

		// Bind default fb (number 0) so that we render normally next time

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Fragment & vertex shaders for drawing a picked primitive.
	ProgramPipeline pickingPipeline = {};
	{
		const char* vShader = R"glshader(
				#version 450 core                             
				                                              
				layout (location=0) in vec3 a_pos;            
				//layout (location=0) uniform mat4 modelViewProjectionMatrix;          

				uniform mat4 modelViewProjectionMatrix;          

				out gl_PerVertex { vec4 gl_Position; };       

				void main()                                   
				{                                             
				    gl_Position = modelViewProjectionMatrix * vec4(a_pos, 1);
                })glshader";

		const char* fShader = R"glshader(
				#version 450 core                          
				layout (location=0)                        
				out vec4 o_color;                          
				void main()                                
				{                                          
				    o_color = vec4(0.90f, 0.90f, 0.90f, 1.0f);
                })glshader";

		pickingPipeline = createProgramPipeline(vShader, fShader);

		pickingPipeline.vertexTransformUniform = glGetUniformLocation(pickingPipeline.vertexShader, "modelViewProjectionMatrix");
	}

	// Fragment & vertex shaders for drawing x and y axis.
	ProgramPipeline axisPipeline = {};
	{
		const char* vShader = R"glshader(
				#version 450 core                             
				                                              
				layout (location=0) in vec2 a_pos;            
				out gl_PerVertex { vec4 gl_Position; };       
				void main()                                   
				{                                             
				    gl_Position = vec4(a_pos, -1.0f, 1.0f);            
                })glshader";

		const char* fShader = R"glshader(
				#version 450 core                          

				uniform ivec2 mousePosition;          
				//uniform int subPixelResolution;          

				layout (location=0)                        
				out vec4 o_color;                          

				// Fixed point sub-pixel snapping.
				// Pixels are centered offset by 0.5.
				//const int fixedPointScaleFactor = 1 << subPixelResolution;

				void main()
				{
					vec4 red = vec4(1.0f, 0.0f, 0.0f, 1.0f);
					vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);
					vec4 green = vec4(0.0f, 1.0f, 0.0f, 1.0f);
					vec4 black = vec4(0.0f, 0.0f, 0.0f, 1.0f);

					float pixelHalfWidth = 0.5f;

					// Normalized othogonal directions.
					vec2 upAxis = vec2(0.0f, 1.0f);
					vec2 rightAxis = vec2(1.0f, 0.0f);

					vec4 inputPosition = vec4(float(mousePosition.x), float(mousePosition.y), 0.0f, 0.0f);

					vec4 inputVector = vec4(gl_FragCoord) - inputPosition;

				    float verticalPixelDelta = dot(upAxis, inputVector.xy);
				    float horizontalPixelDelta = dot(rightAxis, inputVector.xy);

					vec2 fragVectorScaled = (gl_FragCoord.xy);
					vec2 inputVectorScaled = (inputPosition.xy);

					// Upper-left screenspace origin.
					if (mousePosition.x != -1 && mousePosition.y != -1)
					{
						if (horizontalPixelDelta == pixelHalfWidth)
						{
							// Y axis.

							if (fragVectorScaled.y - pixelHalfWidth > inputVectorScaled.y)
							{
								o_color = green;
                                return;
							}
							if (fragVectorScaled.y - pixelHalfWidth == inputVectorScaled.y)
							{
								o_color = white;
                                return;
							}
							else
                            {
                                discard;
                            }
						}
						if (verticalPixelDelta == pixelHalfWidth)
						{
							// X axis.

							if (fragVectorScaled.x - pixelHalfWidth < inputVectorScaled.x)
							{
								o_color = red;
                                return;
							}
							else
                            {
                                discard;
                            }
						}
					}
                    else
                    {
                        discard;
                    }
                })glshader";

		axisPipeline = createProgramPipeline(vShader, fShader);

		axisPipeline.mousePositionUniform = glGetUniformLocation(axisPipeline.fragmentShader, "mousePosition");
	}

	{
		GLint subPixelResolution = 0;

		glGetIntegerv(GL_SUBPIXEL_BITS, &subPixelResolution);

		assert(glGetError() == GL_NO_ERROR);
		assert(subPixelResolution != 0);

		printf("Using subpixel resolution: %d\n", subPixelResolution);

		//assert(glIsProgramPipeline(cubeShader.axisProgram));

		//glUseProgram(cubeShader.axisProgram);
		bindProgramPipeline(axisPipeline);

		cubeShader.subPixelResolutionUniform = glGetUniformLocation(axisPipeline.fragmentShader, "subPixelResolution");

		glProgramUniform1i(axisPipeline.fragmentShader, cubeShader.subPixelResolutionUniform, subPixelResolution);

		assert(glGetError() == GL_NO_ERROR);
	}


	// Setup global GL state
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);
	}

	// set to FALSE to disable vsync
	BOOL vsync = TRUE;
	wglSwapIntervalEXT(vsync ? 1 : 0);

	// show the window
	ShowWindow(window, SW_SHOWDEFAULT);

	LARGE_INTEGER freq, c1;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&c1);

	//float angle = 0;

	for (;;)
	{
		// process all incoming Windows messages
		MSG msg;
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			continue;
		}

		LARGE_INTEGER c2;
		QueryPerformanceCounter(&c2);
		float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
		c1 = c2;

		const Position cursorPos = getCursorWindowPosition(window, width, height);

		int mouseUniform[] = {-1, -1};

		if (cursorPos.x >= 0 && cursorPos.y >= 0 && cursorPos.x < width && cursorPos.y < height)
		{
			if (isCursorPosChanged(cursorPos))
			{
				print("Cursor position in client window: [%d, %d]\n", cursorPos.x, cursorPos.y);
			}

			mouseUniform[0] = cursorPos.x;
			mouseUniform[1] = (height - 1) - cursorPos.y;
		}

		// Set mouse position uniform.
		glProgramUniform2iv(axisPipeline.fragmentShader, axisPipeline.mousePositionUniform, 1, mouseUniform);

		unsigned static int counter = 0;

		drawCubeShaderToTexture(cubeShader, width, height, counter, rttFramebuffer);
		PixelBufferData rttTexels = readFromTextureCube(cursorPos.x, cursorPos.y, width, height, rttFramebuffer);

		drawTexturedCubeShaderToOutput(cubeShader, width, height, counter);

		++counter;

		if (globalIsMouseButtonDown && rttTexels.objectID == 1)
		{
			if (isPrimitiveIDChanged(rttTexels.primitiveID))
			{
				print("Primitive ID: %d\n", rttTexels.primitiveID);
			}

			// Draw selected primitive.
			bindProgramPipeline(pickingPipeline);

			// Set vertex transform uniform.
			glProgramUniformMatrix4fv(pickingPipeline.vertexShader, pickingPipeline.vertexTransformUniform, 1, GL_FALSE, &cubeShader.modelViewProjection.data[0][0]);

			glBindVertexArray(cubeShader.cubePickingVAO);

			glDisable(GL_DEPTH_TEST);

			drawPrimitive(rttTexels.primitiveID);
		}

		if (globalIsMouseButtonDown)
		{
			// Draw coordinate-axis.
			bindProgramPipeline(axisPipeline);
			glBindVertexArray(quadVAO);
			glEnable(GL_DEPTH_TEST);
			drawQuad();
		}

		// Swap the buffers to show output.
		if (!SwapBuffers(dc))
		{
			FatalError("Failed to swap OpenGL buffers!");
		}
	}
}
