#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <string>
#include <stdexcept>
#include <optional>
#include <stack>
#include <functional>

#include <vulkan/vk_enum_string_helper.h>

#include <glm/glm.hpp>

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 colour;
	glm::vec2 texCoord;

	// A vertex binding describes at which rate to load data from memory throughout the vertices. It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		/*
		VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
		VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
		*/

		return bindingDescription;
	}

	// As the function prototype indicates, there are going to be two of these structures.An attribute description struct describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description.We have two attributes, position and color, so we need two attribute description structs.
	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, colour);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool AllFamiliesAvailable() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class DeletionStack
{
private:
	std::stack<std::function<void(void)>> deletionFunctions;
public:
	void AddDeletor(std::function<void(void)>&& deletionFunction)
	{
		deletionFunctions.push(std::move(deletionFunction));
	}

	void RunDeletors()
	{
		while (!deletionFunctions.empty())
		{
			std::invoke(deletionFunctions.top());
			deletionFunctions.pop();
		}
	}

	void Clear()
	{
		while (deletionFunctions.size() > 0)
		{
			deletionFunctions.pop();
		}
	}
};


class VkRenderer
{
private:
	static constexpr uint32_t WIDTH = 800;

	static constexpr uint32_t HEIGHT = 600;

	const std::string MODEL_PATH = "resources/models/viking_room.obj";
	const std::string TEXTURE_PATH = "textures/viking_room.png";


	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	const std::vector<std::string> _validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> _deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	static constexpr bool _enableValidationLayers = false;
#else
	static constexpr bool _enableValidationLayers = true;
#endif

	GLFWwindow* _window;

	DeletionStack _mainDeletionStack;

	DeletionStack _swapChainDeletionStack;

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugMessenger;

	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;

	VkSurfaceKHR _surface;
	VkSwapchainKHR _swapChain;
	std::vector<VkImage> _swapChainImages;
	VkFormat _swapChainImageFormat;
	VkExtent2D _swapChainExtent;
	std::vector<VkImageView> _swapChainImageViews;

	std::vector<VkFramebuffer> _swapChainFramebuffers;

	VkDescriptorSetLayout _descriptorSetLayout;

	std::vector<VkBuffer> _uniformBuffers;
	std::vector<VkDeviceMemory> _uniformBuffersMemory;
	std::vector<void*> _uniformBuffersMapped;
	VkDescriptorPool _descriptorPool;
	std::vector<VkDescriptorSet> _descriptorSets;

	VkPipelineLayout _pipelineLayout;
	VkRenderPass _renderPass;
	VkPipeline _graphicsPipeline;

	VkCommandPool _commandPool;
	std::vector<VkCommandBuffer> _commandBuffers;

	std::vector<VkSemaphore> _imageAvailableSemaphores;
	std::vector<VkSemaphore> _renderFinishedSemaphores;
	std::vector<VkFence> _inFlightFences;

	uint32_t _currentFrame = 0;

	bool _framebufferResized = false;

	VkBuffer _vertexBuffer;
	VkDeviceMemory _vertexBufferMemory;

	VkBuffer _indexBuffer;
	VkDeviceMemory _indexBufferMemory;

	std::vector<Vertex> _vertices;

	std::vector<uint32_t> _indices;

	VkImage _textureImage;
	VkDeviceMemory _textureImageMemory;
	VkImageView _textureImageView;
	VkSampler _textureSampler;

	VkImage _depthImage;
	VkDeviceMemory _depthImageMemory;
	VkImageView _depthImageView;

public:
	/// @brief Public method that consumers of this renderer require to run the render loop
	void run() {
		InitWindow();
#ifdef _DEBUG
		PrintDebugInfo();
#endif
		InitVulkan();
		MainLoop();
		Cleanup();
	}

	void SetFramebufferResized(bool resized) { _framebufferResized = resized; }
private:
	/// @brief Initialises the window to which we will be rendering
	void InitWindow();

	/// @brief Initialises all Vulkan resources, structures etc. that are required to start rendering
	void InitVulkan();

	/// 
	void PickPhysicalDevice();

	void CreateLogicalDevice();

	void CreateSurface();

	/// @brief Cleans up all resources that need to be manually managed
	void Cleanup();

	/// @brief Runs the main rendering loop of our renderer
	void MainLoop();

	void DrawFrame();

	/// @brief Creates the Vulkan instance from which all further Vulkan resources will be created/allocated/used etc.
	void CreateInstance();

	void CleanupSwapChain();

	void RecreateSwapChain();

	void CreateSwapChain();

#pragma region SwapchainHelpers
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
#pragma endregion

	void CreateImageViews();

	void CreateRenderPass();

	void CreateGraphicsPipeline();

#pragma region GraphicsPipelineHelpers
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
#pragma endregion

	void CreateFramebuffers();

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void CreateVertexBuffer();

	void CreateIndexBuffer();

	void CreateCommandPool();

	void CreateCommandBuffers();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkCommandBuffer BeginSingleTimeCommands();

	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void CreateTextureImage();

	void CreateTextureImageView();

	void CreateTextureSampler();

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	bool HasStencilComponent(VkFormat format);

	void CreateDepthResources();

	VkFormat FindDepthFormat();

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	void CreateSyncObjects();

	void CreateDescriptorSetLayout();

	void CreateUniformBuffers();

	void CreateDescriptorPool();

	void CreateDescriptorSets();

	void UpdateUniformBuffer(uint32_t currentImage);

	void LoadModel();

	/// @brief Prints out assorted info that might be useful when debugging the renderer
	void PrintDebugInfo() const;

	std::optional<std::string> CheckValidationLayerSupport();

	void SetupDebugMessenger();

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	bool IsDeviceSuitable(VkPhysicalDevice device);

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	/// @brief Gets all extensions required by this renderer
	/// @return Vector of strings containing the names of the extensions
	std::vector <std::string> GetRequiredExtensions() const;

	/// @brief Wrapper method to simplify checking the returned result of any Vulkan function that returns a status code. 
	/// If the result is not successful, an exception is thrown in debug mode, else it is just ignored
	/// @param result The result of a vkFunctionCall 
	/// @param action The "action" string. e.g. "create instance", "allocate image memory" etc.
	static void VK_CHECK_RESULT(VkResult result, const std::string& action)
	{
#ifdef _DEBUG 
		if (result != VK_SUCCESS) {
			// Using string_VkResult to convert the result code into its string equivalent
			throw std::runtime_error("failed to " + action + "!. Error: " + string_VkResult(result));
		}
#endif
	}


};


