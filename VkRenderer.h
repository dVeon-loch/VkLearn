#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <stdexcept>

#include <vulkan/vk_enum_string_helper.h>

class VkRenderer
{
private:
    static constexpr uint32_t WIDTH = 800;

    static constexpr uint32_t HEIGHT = 600;

    GLFWwindow* _window;

    VkInstance _instance;

public:
    /// @brief Public method that consumers of this renderer require to run the render loop
    void run() {
        InitWindow();
        InitVulkan();
#ifdef _DEBUG
        PrintDebugInfo();
#endif
        MainLoop();
        Cleanup();
    }
private:
    /// @brief Initialises the window to which we will be rendering
    void InitWindow();
    
    /// @brief Initialises all Vulkan resources, structures etc. that are required to start rendering
	void InitVulkan();

    /// @brief Cleans up all resources that need to be manually managed
	void Cleanup();

    /// @brief Runs the main rendering loop of our renderer
    void MainLoop();

    /// @brief Creates the Vulkan instance from which all further Vulkan resources will be created/allocated/used etc.
    void CreateInstance();
    
    /// @brief Prints out assorted info that might be useful when debugging the renderer
    void PrintDebugInfo() const;

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

