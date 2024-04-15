#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

class GUI {
private:
    /** Window to render to */
    GLFWwindow *window;
    /** Video mode of the window */
    const GLFWvidmode *mode;
    /** Settings popup window */
    bool show_settings_window = false;
    /** About popup window */
    bool show_about_window = false;
    /** Window width */
    int window_width = 1280;
    /** Window height */
    int window_height = 720;
    /** Fullscreen flag */
    bool fullscreen = false;
    /** Font */
    ImFont *font = nullptr;
    /** Font path */
    constexpr static const char *font_path = "../src/cpp_indexer/gui/font.ttf";
    /** Font size */
    float font_size = 20.0f;
    /** Font scale */
    float font_scale = 1.0f;

    /**
     * Initialize the visualization (OpenGL, GLFW, GLEW, ImGui, ImPlot, callbacks, videomode...)
     */
    void init();
    /**
     * Render the visualization (ImGui, ImPlot)
     * This is the main loop
     */
    void render();
    /**
     * Cleanup the visualization (OpenGL, GLFW, GLEW, ImGui, ImPlot)
     */
    void cleanup();

    /**
     * GLFW error callback
     * @param error Error code
     * @param description Error description
     */
    static void glfw_error_callback(int error, const char *description);

    int current_index = 0;
    std::vector<std::string> indices = {"index1"};
    int current_field = 0;
    int current_model = 0;
    int k_best = 3;
    bool detect_language = false;

    char query[256] = "";

public:
    /**
     * Default constructor, calls init()
     */
    GUI();
    /**
     * Default destructor, calls cleanup()
     */
    ~GUI();

    /**
     * Main loop, calls render()
     */
    void run();
};
