#include <cstdint>

#include <list>

#include "GLFW/glfw3.h"

namespace raunnes {
class Window {
public:
    Window();
    ~Window();

public:
    void Hide();
    void Show();

    bool Valid();

private:
    GLFWwindow* m_glfwWindow;


public:
    static bool ShouldClose();

private:
    static std::list<Window*> m_sWindows;
};
}