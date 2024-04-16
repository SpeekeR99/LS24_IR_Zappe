#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "IndexHandler.h"

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
    std::vector<std::string> indices = {};
    int current_field = 0;
    int current_model = 0;
    int k_best = 3;
    bool detect_language = false;

    std::string query;
    std::string query_lang;

    char new_index_name[256] = "";
    char data_path[256] = "../data";
    int current_doc_id = 0;
    std::string current_doc_title;
    std::string current_doc_toc;
    std::string current_doc_h1;
    std::string current_doc_h2;
    std::string current_doc_h3;
    std::string current_doc_content;
    char url[256] = "";

    std::vector<Indexer> indexers = {};

    int total_results = 0;
    std::vector<Document> search_results = {};
    const int snippet_window_size = 30;
    std::vector<std::string> result_snippets = {};
    std::vector<std::vector<int>> highlight_indices = {};

    bool phrase_search = false;
    bool proximity_search = false;
    int proximity = 3;

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
