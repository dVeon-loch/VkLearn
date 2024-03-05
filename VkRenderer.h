#pragma once
class VkRenderer
{
private:
    static constexpr uint32_t WIDTH = 800;

    static constexpr uint32_t HEIGHT = 600;

    GLFWwindow* _window;

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
};

