#include "VkRenderer.h"

#include<cstring>
#include<iostream>

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
	CreateInstance();
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

void VkRenderer::CreateInstance()
{
	// Optional info for the driver
	VkApplicationInfo appInfo{}; // Curly braces very important here, they initialize all fields to 0/null/a defined value
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0; // TODO: Update version

	// Mandatory info to create the instance
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	std::vector<std::string> requiredExtensions = GetRequiredExtensions();

	std::vector<const char*> extensionsCString;
	extensionsCString.reserve(requiredExtensions.size());

	for (const auto& extension : requiredExtensions)
	{
		extensionsCString.push_back(extension.c_str());
	}


	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensionsCString.data();
	instanceCreateInfo.enabledLayerCount = 0; // TODO: Enable validation layers

	VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance), "create instance");
}

void VkRenderer::PrintDebugInfo()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "Available extensions: " << "\n";
	for(const auto& extension : extensions)
	{
		std::cout << extension.extensionName << ", Version: " << extension.specVersion << "\n";
	}
}

std::vector<std::string> VkRenderer::GetRequiredExtensions()
{
	std::vector<std::string> requiredExtensions;

	/// GLFW EXTENSIONS
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


	for (size_t i = 0; i < glfwExtensionCount; i++)
	{
		const std::string extension(glfwExtensions[i]);
		requiredExtensions.push_back(extension);
	}

	return requiredExtensions;
}
