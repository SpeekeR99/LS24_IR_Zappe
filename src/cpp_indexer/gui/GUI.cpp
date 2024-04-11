#include "GUI.h"

void GUI::glfw_error_callback(int error, const char *description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

GUI::GUI() : window(nullptr), mode(nullptr) {
    this->init();
}

GUI::~GUI() {
    this->cleanup();
}

void GUI::init() {
    /* Initialize the library */
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        throw std::runtime_error("GLFW initialization error");

    /* GL 3.0 + GLSL 330 */
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); /* Disable resizing */
    this->window = glfwCreateWindow(window_width, window_height, "NSES Zappe", nullptr, nullptr);
    if (!this->window) {
        glfwTerminate();
        throw std::runtime_error("GLFW window creation error");
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(this->window);
    mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(this->window, (mode->width - window_width) / 2, (mode->height - window_height) / 2);

    /* Initialize GLEW */
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        throw std::runtime_error("GLEW initialization error");
    }

    /* Setup Dear ImGui context */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    /* Setup Dear ImGui style */
    ImGui::StyleColorsDark();

    /* Print out some info about the graphics drivers */
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;

    /* Setup Platform/Renderer backends */
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    /* Background color */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void GUI::render() {
    /* Poll for and process events */
    glfwPollEvents();

    /* Start the Dear ImGui frame */
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    /* GUI part */
    {
        /* Set up style variables */
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        /* Set up the main GUI configuration window */
        ImGui::Begin("Configuration", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                     ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

        /* Set up the main GUI window position, size and font size */
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2((float) window_width / 2.0f, (float) window_height));
        ImGui::SetWindowFontScale(font_size);

        /* Set up the main GUI menu bar */
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) { /* File menu */
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                    glfwSetWindowShouldClose(window, true);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings")) { /* Settings menu */
                if (ImGui::MenuItem("Graphics Settings")) {
                    show_settings_window = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) { /* Help menu */
                if (ImGui::MenuItem("About")) {
                    show_about_window = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        /* Data section */
        ImGui::SeparatorText("Data");

        /* Neural Network settings section */
        ImGui::SeparatorText("Neural Network settings");

        /* Training section */
        ImGui::SeparatorText("Training");

        ImGui::Separator();

        ImGui::End();

        /* Right side of the GUI, data visualization, loss function */
        ImGui::Begin("Data visuals", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                     ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

        /* Set up the main GUI window position, size and font size */
        ImGui::SetWindowPos(ImVec2((float) window_width / 2.0f, 0));
        ImGui::SetWindowSize(ImVec2((float) window_width / 2.0f, (float) window_height));
        ImGui::SetWindowFontScale(font_size);

        /* TabBar */
        if (ImGui::BeginTabBar("##TabBar")) {
            /* Data tab */
            if (ImGui::BeginTabItem("Data")) {
                ImGui::EndTabItem();
            }
            /* Loss function tab */
            if (ImGui::BeginTabItem("Loss function")) {
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        /* Reset the style */
        ImGui::PopStyleVar(2);
    }

    /* Settings window */
    {
        if (show_settings_window) {
            /* Create a window */
            ImGui::Begin("Graphics Settings", &show_settings_window,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            /* Set the window size */
            ImGui::SetWindowSize(ImVec2(600, 400));

            /* Window settings section */
            ImGui::SeparatorText("Window Settings");

            /* Set the size of the window and the position of the window */
            if (ImGui::Button("Set 1280x720")) {
                window_width = 1280;
                window_height = 720;
                glfwSetWindowSize(window, window_width, window_height);
                glfwSetWindowPos(window, (mode->width - window_width) / 2, (mode->height - window_height) / 2);
            }
            ImGui::SameLine();
            if (ImGui::Button("Set 1600x900")) {
                window_width = 1600;
                window_height = 900;
                glfwSetWindowSize(window, window_width, window_height);
                glfwSetWindowPos(window, (mode->width - window_width) / 2, (mode->height - window_height) / 2);
            }
            ImGui::SameLine();
            if (!fullscreen) {
                if (ImGui::Button("Fullscreen")) {
                    window_width = mode->width;
                    window_height = mode->height;
                    fullscreen = true; // Set fullscreen to true
                }
            } else {
                if (ImGui::Button("Windowed")) {
                    window_width = 1280;
                    window_height = 720;
                    glfwSetWindowMonitor(window, nullptr, 0, 0, window_width, window_height, 0);
                    glfwSetWindowPos(window, (mode->width - window_width) / 2, (mode->height - window_height) / 2);
                    fullscreen = false; // Set fullscreen to false
                }
            }

            /* Set the font size */
            if (ImGui::InputFloat("Font Size", &font_size, 0.1f, 0.3f, "%.1f")) {
                // On change, clamp the value between 1.0 and 2.0
                if (font_size < 1.0f) font_size = 1.0f;
                if (font_size > 2.0f) font_size = 2.0f;
            }
            ImGui::SetWindowFontScale(font_size);

            /* Colors section */
            ImGui::SeparatorText("Colors");
            static int style_idx = 0; // Overall style
            if (ImGui::Combo("Style", &style_idx, "Dark\0Light\0Classic\0")) {
                switch (style_idx) {
                    case 0:
                        ImGui::StyleColorsDark();
                        break;
                    case 1:
                        ImGui::StyleColorsLight();
                        break;
                    case 2:
                        ImGui::StyleColorsClassic();
                        break;
                    default:
                        break;
                }
            }
            ImGui::End();
        }
    }

    /* About Window */
    {
        if (show_about_window) {
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always,
                                    ImVec2(0.5f, 0.5f)); // Center the window
            ImGui::Begin("About", &show_about_window,
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse);
            ImGui::SetWindowFontScale(font_size); // Set the font size
            ImGui::Text("This application was made by:\nDominik Zappe");
            ImGui::Separator();
            ImGui::Text("Application serves as Semestral Project for IR course");
            ImGui::End();
        }
    }

    /* ImGui Rendering */
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this->window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    /* Swap front and back buffers */
    glfwSwapBuffers(this->window);
    glfwSwapInterval(1); /* Enable vsync */
}

void GUI::cleanup() {
    /* Cleanup */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void GUI::run() {
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        this->render();
    }
}
