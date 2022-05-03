#pragma once

#include <string>
#include "vector2.hpp"

struct GLFWwindow;

class Window
{
public:
	static void PollEvents();
	static void Init();

public:
	Window(const Vector2i& size, const std::string& title);
	~Window();

	operator bool() const;

	void Close();
	void Clear(float r, float g, float b, float a);
	void SwapBuffers();

private:
	GLFWwindow* handle;
};