#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include<vector>
#include<string>
#include <stdexcept>

#include <vulkan/vk_enum_string_helper.h >

class VkRenderer
{
private:
    static constexpr uint32_t WIDTH = 800;

    static constexpr uint32_t HEIGHT = 600;

    GLFWwindow* _window;

    VkInstance _instance;

public:
    void run() {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

    void InitWindow();

	void InitVulkan();

	void Cleanup();

    void MainLoop();

private:
    void CreateInstance();
    std::vector <std::string> GetRequiredExtensions();

    static void VK_CHECK_RESULT(VkResult result, std::string action)
    {
#ifdef _DEBUG 
        if (result != VK_SUCCESS) {
            // Using string_VkResult to convert the result code into its string equivalent
            throw std::runtime_error("failed to " + action + "!. Error: " + string_VkResult(result) );
        }
#endif
    }
};

