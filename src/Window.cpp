#include "Window.h"
#include <algorithm>

namespace raunnes {

std::list<Window*> Window::m_sWindows;


Window::Window() :
    m_glfwWindow(nullptr) {

    m_glfwWindow = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (m_glfwWindow) {
        glfwMakeContextCurrent(m_glfwWindow);
    }

    m_sWindows.emplace_back(this);
}

Window::~Window() {
    m_sWindows.remove(this);
}

bool Window::Valid() {
    return m_glfwWindow && !glfwWindowShouldClose(m_glfwWindow);
}

bool Window::ShouldClose() {
    bool close = true;

    std::for_each(m_sWindows.begin(),
        m_sWindows.end(),
        [&close](Window* w) { close &= w->Valid(); });

    return close;
}

}