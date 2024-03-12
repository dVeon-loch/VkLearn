#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <optional>

#include <vulkan/vk_enum_string_helper.h>

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool AllFamiliesAvailable() {
        return graphicsFamily.has_value();
    }
};

class VkRenderer
{
private:
    static constexpr uint32_t WIDTH = 800;

    static constexpr uint32_t HEIGHT = 600;

    const std::vector<std::string> _validationLayers = {
    "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    static constexpr bool _enableValidationLayers = false;
#else
    static constexpr bool _enableValidationLayers = true;
#endif

    GLFWwindow* _window;

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;

    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkQueue _graphicsQueue;

    VkSurfaceKHR _surface;

public:
    /// @brief Public method that consumers of this renderer require to run the render loop
    void run() {
        InitWindow();
#ifdef _DEBUG
        PrintDebugInfo();
#endif
        CreateSurface();
        InitVulkan();
        MainLoop();
        Cleanup();
    }
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

    /// @brief Creates the Vulkan instance from which all further Vulkan resources will be created/allocated/used etc.
    void CreateInstance();
    
    /// @brief Prints out assorted info that might be useful when debugging the renderer
    void PrintDebugInfo() const;

    std::optional<std::string> CheckValidationLayerSupport();

    void SetupDebugMessenger();

    /// @brief Gets all extensions required by this renderer
    /// @return Vector of strings containing the names of the extensions
    std::vector <std::string> GetRequiredExtensions() const;

    /// @brief Wrapper method to simplify checking the returned result of any Vulkan function that returns a status code. 
    /// If the result is not successful, an exception is thrown in debug mode, else it is just ignored
    /// @param result The result of a vkFunctionCall 
    /// @param action The "action" string. e.g. "create instance", "allocate image memory" etc.
    static void VK_CHECK_RESULT(VkResult result, const std::string &action)
    {
#ifdef _DEBUG 
        if (result != VK_SUCCESS) {
            // Using string_VkResult to convert the result code into its string equivalent
            throw std::runtime_error("failed to " + action + "!. Error: " + string_VkResult(result) );
        }
#endif
    }

   
};

