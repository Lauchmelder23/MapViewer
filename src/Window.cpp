#include "Window.hpp"

#include <stdexcept>
#include <glad/glad.h>
#include <glfw/glfw3.h>

void Window::PollEvents()
{
	glfwPollEvents();
}

void Window::Init()
{
	glfwInit();
}

Window::Window(const Vector2i& size, const std::string& title)
{
	handle = glfwCreateWindow(size.x, size.y, title.c_str(), NULL, NULL);
	if (!handle)
		throw std::runtime_error("Failed to create window");

	glfwMakeContextCurrent(handle);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to load GL loader");

	glViewport(0, 0, size.x, size.y);
}

Window::~Window()
{
	if(handle)
		glfwDestroyWindow(handle);
}

Window::operator bool() const
{
	return !(bool)glfwWindowShouldClose(handle);
}

void Window::Close()
{
	glfwSetWindowShouldClose(handle, true);
}

void Window::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Window::SwapBuffers()
{
	glfwSwapBuffers(handle);
}
