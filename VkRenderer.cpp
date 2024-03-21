#include "VkRenderer.h"

#include<cstring>
#include<iostream>
#include<iomanip>
#include<format>
#include<set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

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
	CreateSwapChain();
	CreateImageViews();
	CreateGraphicsPipeline();
}

bool VkRenderer::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	QueueFamilyIndices indices = FindQueueFamilies(device);

	const bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapchainDetails = QuerySwapChainSupport(device);
		swapChainAdequate = !swapchainDetails.formats.empty() && !swapchainDetails.presentModes.empty();
	}

	// Pick your dedicated GPU
	return indices.AllFamiliesAvailable() && extensionsSupported  && swapChainAdequate && deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
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

	createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
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
	for (const auto &imageView : _swapChainImageViews) {
		vkDestroyImageView(_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(_device, _swapChain, nullptr);

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
	appInfo.apiVersion = VK_API_VERSION_1_2; // TODO: Update version

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

void VkRenderer::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	// Request an extra image so we don't have to wait on the driver to complete internal operations before we can acquire another image to render to
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// 0 is a special value that means that there is no maximum
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		// Clamp imageCount to the max if we have exceeded the max
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // This is always 1 unless you are developing a stereoscopic 3D application
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // We're going to render directly to them, which means that they're used as color attachment

	QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	/*
    VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family. This option offers the best performance.
    VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers.
	*/
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	// We can specify that a certain transform should be applied to images in the swap chain if it is supported (supportedTransforms in capabilities), like a 90 degree clockwise rotation or horizontal flip. To specify that you do not want any transformation, simply specify the current transformation
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system.
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // If the clipped member is set to VK_TRUE then that means that we don't care about the color of pixels that are obscured, for example because another window is in front of them
	createInfo.oldSwapchain = VK_NULL_HANDLE; // Will be used when we get to resizing the window

	VK_CHECK_RESULT(vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain),"create swapchain");

	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
	_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

	_swapChainImageFormat = surfaceFormat.format;
	_swapChainExtent = extent;
}

SwapChainSupportDetails VkRenderer::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails swapchainSupportDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &swapchainSupportDetails.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
	if(formatCount != 0)
	{
		swapchainSupportDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, swapchainSupportDetails.formats.data());
	}


	uint32_t presentModesCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModesCount, nullptr);
	if (presentModesCount != 0)
	{
		swapchainSupportDetails.presentModes.resize(presentModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModesCount, swapchainSupportDetails.presentModes.data());
	}

	return swapchainSupportDetails;
}

VkSurfaceFormatKHR VkRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VkRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}
// TODO: Explain this method
VkExtent2D VkRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// if the extent is the special value, (0xFFFFFFFF, 0xFFFFFFFF), the surface size
	// can be determined by the extent of the swapchain targeting this surface,
	// else it must stay the same value as the current extent
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilities.currentExtent;
	}
	else 
	{
		// The surface current extent is the special value, we need to calculate the 
		// required swapchain extent

		// get the extent in pixels of the glfw window we created
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		// clamp the extent to the limits
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VkRenderer::CreateImageViews()
{
	// We need one image view per image in the swapchain
	_swapChainImageViews.resize(_swapChainImages.size());

	// Each image needs a corresponding  image view, so use a normal for loop to keep track of the index
	for (size_t i = 0; i < _swapChainImages.size(); i++) 
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = _swapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // This is going to be a regular 2D texture
		createInfo.format = _swapChainImageFormat; // The format of the view is the same as the format of the swapchain image

		// Swizzling allows us to swap the channels around, e.g. if we only want the red channel to be kept for some reason
		// We won't need this for now so just leave everything as 'identity'
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// The subresourceRange field describes what the image's purpose is and which part of the image should be accessed.
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Use as a colour target
		createInfo.subresourceRange.baseMipLevel = 0; // base mip level is just the first level.
		createInfo.subresourceRange.levelCount = 1; // Only one level (no mipmap)
		createInfo.subresourceRange.baseArrayLayer = 0; 
		createInfo.subresourceRange.layerCount = 1; // We don't need any layers to this texture
		// If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers. You could then create multiple image views for each image representing the views for the left and right eyes by accessing different layers.

		VK_CHECK_RESULT(vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]), "create image view: "+i);
	}
}

void VkRenderer::CreateGraphicsPipeline()
{
	auto vertShaderCode = vkutil::ReadFile("shaders/vert.spv");
	auto fragShaderCode = vkutil::ReadFile("shaders/frag.spv");

	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	// VERTEX STAGE CREATION
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // This create info is for the vertex stage
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main"; // We can specify any entry point into the shaders, stick with main

	// FRAGMENT STAGE CREATION
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // This create info is for the fragment stage
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };




	vkDestroyShaderModule(_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}

VkShaderModule VkRenderer::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); // We need a reinterpret_cast here because we are casting between pointer types
	
	VkShaderModule shaderModule;
	VK_CHECK_RESULT(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule), "create shader module");

	return shaderModule;
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
