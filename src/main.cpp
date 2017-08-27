#include <cstdlib>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <core/Assert.h>

#include <core/File.h>
#include <core/Log.h>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#define GL_ASSERT(f) f; ASSERT(glGetError() == GL_NO_ERROR) 

GLuint LoadAndCompileShader(const char* _path, GLuint _type)
{
	File shaderFile(_path);
	if (!shaderFile.load())
	{
		shaderFile.release();
		LOG_ERROR("Failed to load shader \"%s\"", _path);
		return GL_INVALID_ENUM;
	}

	GLuint shader = glCreateShader(_type);
	if (shader == GL_INVALID_ENUM)
	{
		shaderFile.release();
		return GL_INVALID_ENUM;
	}

	const char* content = shaderFile.getContent();
	GL_ASSERT(glShaderSource(shader, 1, &content, nullptr));
	GL_ASSERT(glCompileShader(shader));
	GLsizei shaderInfoLength;
	char shaderInfo[512];
	glGetShaderInfoLog(shader, 512, &shaderInfoLength, shaderInfo);
	ASSERT(shaderInfoLength < 512);
	if (shaderInfoLength == 0)
		strcpy(shaderInfo, "OK");
	LOG("Compiling shader \"%s\"...\n%s", _path, shaderInfo);

	shaderFile.release();
	return shader;
}

static GLuint s_vertexShader = 0;
static GLuint s_fragmentShader = 0;
static GLuint s_shaderProgram = 0;

static GLint s_aPosition_location = -1;
static GLint s_uScreenSize_location = -1;

static void ReloadShaders()
{
	if (s_shaderProgram != 0) { glDeleteProgram(s_shaderProgram); s_shaderProgram = 0; }
	if (s_vertexShader != 0) { glDeleteShader(s_vertexShader); s_vertexShader = 0; }
	if (s_fragmentShader != 0) { glDeleteShader(s_fragmentShader); s_fragmentShader = 0; }

	s_vertexShader = LoadAndCompileShader("shaders/quad_texture.vs", GL_VERTEX_SHADER);
	s_fragmentShader = LoadAndCompileShader("shaders/quad_texture.fs", GL_FRAGMENT_SHADER);
	GL_ASSERT(s_shaderProgram = glCreateProgram());
	GL_ASSERT(glAttachShader(s_shaderProgram, s_vertexShader));
	GL_ASSERT(glAttachShader(s_shaderProgram, s_fragmentShader));
	GL_ASSERT(glLinkProgram(s_shaderProgram));

	GL_ASSERT(s_aPosition_location = glGetAttribLocation(s_shaderProgram, "aPosition"));
	GL_ASSERT(s_uScreenSize_location = glGetUniformLocation(s_shaderProgram, "uScreenSize"));

	GL_ASSERT(glEnableVertexAttribArray(s_aPosition_location));
	GL_ASSERT(glVertexAttribPointer(s_aPosition_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0));
}

static const struct { float x, y; } s_quadVertices[6] =
{
	{ -1.f, -1.f },
	{  1.f, -1.f },
	{  1.f,  1.f },
	{ -1.f, -1.f },
	{  1.f,  1.f },
	{ -1.f,  1.f }
};

int main(int _argc, char** _argv)
{

	if (!glfwInit())
	{
		LOG_ERROR("Failed to initialize glfw.");
		return EXIT_FAILURE;
	}

	GLFWwindow* window = glfwCreateWindow(800, 600, "Raytracer", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	if (gl3wInit()) {
		LOG_ERROR("Failed to initialize OpenGL.");
		return EXIT_FAILURE;
	}

	if (!ImGui_ImplGlfwGL3_Init(window, true))
	{
		LOG_ERROR("Failed to initialize ImGui.");
		return EXIT_FAILURE;
	}

	GLuint renderTexture;
	float* textureBuffer = nullptr;
	size_t textureBufferSize = 0;
	GL_ASSERT(glGenTextures(1, &renderTexture));
	GL_ASSERT(glActiveTexture(GL_TEXTURE0));
	GL_ASSERT(glBindTexture(GL_TEXTURE_2D, renderTexture));
	GL_ASSERT(glEnable(GL_TEXTURE_2D));
	GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	GLuint quadBuffer;
	GL_ASSERT(glGenBuffers(1, &quadBuffer));
	GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, quadBuffer));
	GL_ASSERT(glBufferData(GL_ARRAY_BUFFER, sizeof(s_quadVertices), s_quadVertices, GL_STATIC_DRAW));

	ReloadShaders();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		{
			if (ImGui::Button("Reload Shaders"))
			{
				ReloadShaders();
			}
		}

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		size_t newBufferSize = width * height * 3;
		if (textureBufferSize < newBufferSize)
		{
			delete[] textureBuffer;
			textureBuffer = new float[newBufferSize];
			textureBufferSize = newBufferSize;
		}
		for (size_t y = 0; y < height; ++y)
		for (size_t x = 0; x < width; ++x)
		{
			size_t base = (y * width + x) * 3;
			textureBuffer[base + 0] = 0.f;
			textureBuffer[base + 1] = (float)x / width;
			textureBuffer[base + 2] = (float)y / height;
		}
		GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, textureBuffer));
		
		GL_ASSERT(glClearColor(1.0f, 1.0f, 0.0f, 1.0f));
		GL_ASSERT(glViewport(0, 0, width, height));
		GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT));

		GL_ASSERT(glUseProgram(s_shaderProgram));
		glm::vec2 screenSize(width, height);
		screenSize = screenSize;
		GL_ASSERT(glUniform2fv(s_uScreenSize_location, 1, (GLfloat*)&screenSize));
		GL_ASSERT(glDrawArrays(GL_TRIANGLES, 0, 6));

		ImGui::Render();
		glfwSwapBuffers(window);
	}

	ImGui_ImplGlfwGL3_Shutdown();

	glfwTerminate();
	return EXIT_SUCCESS;
}