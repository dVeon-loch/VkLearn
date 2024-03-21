#pragma once

#include "VkRenderer.h"


namespace vkutil
{
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    std::vector<char> ReadFile(const std::string& filename);
}