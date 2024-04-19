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
#include "Const.h"

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

    /** Current chosen index */
    int current_index = 0;
    /** Index names */
    std::vector<std::string> indices = {};
    /** Current chosen field */
    int current_field = 0;
    /** Current chosen model */
    int current_model = 0;
    /** K best results */
    int k_best = 3;
    /** Detect language (query) */
    bool detect_language = false;

    /** Query */
    std::string query;
    /** Query language */
    std::string query_lang;

    /** Index name */
    char new_index_name[256] = "";
    /** Data path */
    char data_path[256] = "../data";
    /** Current document ID */
    int current_doc_id = 0;
    /** Current document title */
    std::string current_doc_title;
    /** Current document TOC */
    std::string current_doc_toc;
    /** Current document H1 */
    std::string current_doc_h1;
    /** Current document H2 */
    std::string current_doc_h2;
    /** Current document H3 */
    std::string current_doc_h3;
    /** Current document content */
    std::string current_doc_content;
    /** URL to download from */
    char url[256] = "";

    /** Indexers */
    std::vector<Indexer> indexers = {};

    /** Number of results */
    int total_results = 0;
    /** Search results */
    std::vector<Document> search_results = {};
    /** Snippet window size */
    const int snippet_window_size = 30;
    /** Snippets */
    std::vector<std::string> result_snippets = {};
    /** Highlighted indices */
    std::vector<std::vector<int>> highlight_indices = {};

    /** Phrase search flag */
    bool phrase_search = false;
    /** Proximity search flag */
    bool proximity_search = false;
    /** Proximity */
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
