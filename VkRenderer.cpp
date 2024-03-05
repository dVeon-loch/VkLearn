#include "VkRenderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void VkRenderer::InitWindow()
{
	// Initialise GLFW and OpenGL
	glfwInit();
	// Tell GLFW not to create an OpenGL context since we don't need one
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// Don't worry about resizing the window for now, just disable it
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_window = glfwCreateWindow(WIDTH, HEIGHT, "VkLearn", nullptr, nullptr);
}

void VkRenderer::InitVulkan()
{
	// TODO
}

void VkRenderer::Cleanup()
{
	glfwDestroyWindow(_window);

	glfwTerminate();
}

void VkRenderer::MainLoop()
{
	while (!glfwWindowShouldClose(_window)) {
		glfwPollEvents();
	}
}
