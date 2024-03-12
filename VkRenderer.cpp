#include "VkRenderer.h"

#include<cstring>
#include<iostream>
#include<iomanip>
#include<format>
#include<set>

#include "VkUtils.h"

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void VkRenderer::InitWindow()
{
	// Initialise GLFW and OpenGL
	glfwInit();
	// Tell GLFW not to create an OpenGL context since we don't need one
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// Don't worry about resizing the window for now, just disable it
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	// Store a handle to our window in our class
	_window = glfwCreateWindow(WIDTH, HEIGHT, "VkLearn", nullptr, nullptr);
}

void VkRenderer::InitVulkan()
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
}

bool VkRenderer::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	QueueFamilyIndices indices = FindQueueFamilies(device);

	const bool extensionsSupported = CheckDeviceExtensionSupport(device);

	// Pick your dedicated GPU
	return indices.AllFamiliesAvailable() && extensionsSupported && deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool VkRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices VkRenderer::FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
		// NOTE: These will probably be the same queue, but if you want to guarantee that, you can add
		// logic in during physical device selection and here to ensure that
		if (presentSupport)
		{
			indices.presentFamily = i;
		}
		if (indices.AllFamiliesAvailable()) {
			break;
		}
		i++;
	}

	return indices;
}

void VkRenderer::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	//If there are 0 devices with Vulkan support then there is no point going further.
	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	//Otherwise we can now allocate an array to hold all of the VkPhysicalDevice handles.
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

	for (const auto& device : devices) 
	{
		if (IsDeviceSuitable(device)) 
		{
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void VkRenderer::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);

	// set ensures that we only have one queue family index if the queues are actually the same
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(uniqueQueueFamilies.size());

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = 0;
	// Don't need to do this in newer versions of Vulkan
	/*if (_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
		createInfo.ppEnabledLayerNames = _validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}*/
	VK_CHECK_RESULT(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device), "create logical device");

	// Get handles to queues we need
	//Device queues are implicitly cleaned up when the device is destroyed, so we don't need to do anything in cleanup.
	// Queue index must be less than the number of queues you reserved using the queueCount number above in
	// VkDeviceQueueCreateInfo
	vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
}

void VkRenderer::CreateSurface()
{
	VK_CHECK_RESULT(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface), "create window surface");
}

void VkRenderer::Cleanup()
{
	vkDestroyDevice(_device, nullptr);

	if (_enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(_instance, _surface, nullptr);

	vkDestroyInstance(_instance, nullptr);

	/// GLFW Cleanup
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
	std::optional<std::string> missingLayers = CheckValidationLayerSupport();

	if (_enableValidationLayers && missingLayers.has_value()) {
		throw std::runtime_error("validation layers requested, but not available!\nMissing Layers:\n" + missingLayers.value());
	}

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

	std::vector<const char*> layersCString;
	layersCString.reserve(_validationLayers.size());

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (_enableValidationLayers) {
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());

		for (const auto& layer : _validationLayers)
		{
			layersCString.push_back(layer.c_str());
		}

		instanceCreateInfo.ppEnabledLayerNames = layersCString.data();

		vkutil::PopulateDebugMessengerCreateInfo(debugCreateInfo);
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		instanceCreateInfo.enabledLayerCount = 0;
	}

	VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance), "create instance");
}

void VkRenderer::PrintDebugInfo() const
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported." << "\n";

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "Available extensions: " << "\n";
	for (const auto& extension : extensions)
	{
		std::cout << std::format("\t{:45} Version: {}", extension.extensionName, extension.specVersion) << '\n';
	}
}

std::optional<std::string> VkRenderer::CheckValidationLayerSupport()
{
	std::optional<std::string> returnedLayers = std::nullopt;
	std::string unavailableLayers;

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Loop over all of the layers that we stated we require in _validationLayers
	// For each required layer, loop over the *available* layers of the instance.
	// If the required layer matches any of the available layers, break out and check the next layer
	// If it doesn't, the required layer is not available, so return false.
	// If all layers are found, return true
	for (const auto& layerName : _validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (layerName.compare(layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			unavailableLayers += layerName + '\n';
			returnedLayers = std::make_optional(unavailableLayers);
		}
	}

	return returnedLayers;
}

void VkRenderer::SetupDebugMessenger()
{
	if (!_enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;

	vkutil::PopulateDebugMessengerCreateInfo(debugMessengerCreateInfo);

	VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(_instance, &debugMessengerCreateInfo, nullptr, &_debugMessenger), "create debug messenger");
}

std::vector<std::string> VkRenderer::GetRequiredExtensions() const
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

	/// DEBUG MESSENGER EXTENSION
	if (_enableValidationLayers) {
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	/// ADD MORE EXTENSIONS HERE AS NEEDED

	return requiredExtensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	// This callback allows us to filter out validation layer messages based on various conditions such as severity, message type etc.
	// For now just print everything out
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	// Extra info:
	/*
	The messageType parameter can have the following values:

	VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
	VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake
	VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan
	*/

	// Return false if you don't want to exit the program (always return false)
	return VK_FALSE;
}
