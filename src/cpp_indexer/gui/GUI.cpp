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
    this->window = glfwCreateWindow(window_width, window_height, "Zaklínačský Vyhledávač na IR - Zappe", nullptr, nullptr);
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

    /* Load the font */
    this->font = io.Fonts->AddFontFromFileTTF(font_path, font_size);

    /* Initialize the index */
    for (const auto &entry : std::filesystem::directory_iterator("../index")) {
        indices.emplace_back(entry.path().filename().replace_extension().string());
        Indexer indexer = Indexer();
        IndexHandler::load_index(indexer, entry.path().string());
        indexers.emplace_back(indexer);
    }
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

    /* Set the font */
    ImGui::PushFont(this->font);

    /* GUI part */
    {
        /* Set up style variables */
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        /* Set up the main GUI configuration window */
        ImGui::Begin("Hlavni okno", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                     ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);

        /* Set up the main GUI window position, size and font size */
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2((float) window_width, (float) window_height));
        ImGui::SetWindowFontScale(font_scale);

        /* Set up the main GUI menu bar */
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Soubor")) { /* File menu */
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                    glfwSetWindowShouldClose(window, true);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Nastaveni")) { /* Settings menu */
                if (ImGui::MenuItem("Graficka nastaveni...")) {
                    show_settings_window = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Pomoc")) { /* Help menu */
                if (ImGui::MenuItem("O aplikaci...")) {
                    show_about_window = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        /* TabBar */
        if (ImGui::BeginTabBar("##TabBar")) {
            /* Search tab */
            if (ImGui::BeginTabItem("Vyhledavac")) {
                /* Set up the main GUI configuration window */
                ImGui::Begin("Konfigurace", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2(0, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                if (ImGui::BeginCombo("Index", indices[current_index].c_str())) {
                    for (int i = 0; i < indices.size(); i++) {
                        bool is_selected = (current_index == i);
                        if (ImGui::Selectable(indices[i].c_str(), is_selected))
                            current_index = i;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                const char *field_names[] = {"Vse", "Nadpis", "Obsah"};
                ImGui::ListBox("Hledat v", &current_field, field_names, IM_ARRAYSIZE(field_names));

                const char *model_names[] = {"Vektorovy", "Booleovsky"};
                ImGui::ListBox("Model", &current_model, model_names, IM_ARRAYSIZE(model_names));

                if (current_model == 0) { /* Vector model */
                    ImGui::InputInt("K nejlepsich", &k_best);
                    if (k_best < 1)
                        k_best = 1;
                }

                ImGui::Checkbox("Detekce jazyka\n(dotazu)", &detect_language);

                ImGui::End();

                ImGui::Begin("Vyhledavani", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2((float) window_width / 3, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3 * 2, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 80, ImGui::GetCursorPosY() + 20));

                ImGui::InputText("##Dotaz", query, IM_ARRAYSIZE(query));
                ImGui::SameLine();
                if (ImGui::Button("Hledej")) {

                    /* TODO: Search */
                    std::cout << "Hledam" << std::endl;

                }

                ImGui::End();

                ImGui::EndTabItem();
            }
            /* Indexer tab */
            if (ImGui::BeginTabItem("Indexovani")) {
                /* Set up the Index window */
                ImGui::Begin("Index", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2(0, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                if (ImGui::BeginCombo("Vybrany index", indices[current_index].c_str())) {
                    for (int i = 0; i < indices.size(); i++) {
                        bool is_selected = (current_index == i);
                        if (ImGui::Selectable(indices[i].c_str(), is_selected))
                            current_index = i;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::InputText("Novy index", new_index_name, IM_ARRAYSIZE(new_index_name));
                if (ImGui::Button("Vytvorit")) {
                    indices.emplace_back(new_index_name);
                    current_index = indices.size() - 1;
                }

                ImGui::InputText("Data", data_path, IM_ARRAYSIZE(data_path));
                if (ImGui::Button("Nacist data")) {
                    /* TODO: Load data */
                }

                ImGui::InputInt("ID", &current_doc);
                if (ImGui::Button("Nacist dokument")) {
                    /* TODO: Load document */
                }

                ImGui::End();

                /* Set up the Document window */
                ImGui::Begin("Dokument", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2((float) window_width / 3, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3 * 2, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                /* TODO: Title; TOC; H1; H2; H3; Content; Create/Update button; Delete button */
                ImGui::InputText("Nadpis", &current_doc_title);
                ImGui::InputText("Osnova", &current_doc_toc);
                ImGui::InputText("H1", &current_doc_h1);
                ImGui::InputText("H2", &current_doc_h2);
                ImGui::InputText("H3", &current_doc_h3);
                ImGui::InputTextMultiline("Obsah", &current_doc_content);

                if (ImGui::Button("Vytvorit/aktualizovat")) {
                    /* TODO: Create/Update document */
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                if (ImGui::Button("Smazat")) {
                    /* TODO: Delete document */
                }
                ImGui::PopStyleColor(3);

                ImGui::End();

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
            ImGui::Begin("Graficka nastaveni", &show_settings_window,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            /* Set the window size */
            ImGui::SetWindowSize(ImVec2(600, 400));

            /* Window settings section */
            ImGui::SeparatorText("Nastaveni okna");

            /* Set the size of the window and the position of the window */
            if (ImGui::Button("Nastav na 1280x720")) {
                window_width = 1280;
                window_height = 720;
                glfwSetWindowSize(window, window_width, window_height);
                glfwSetWindowPos(window, (mode->width - window_width) / 2, (mode->height - window_height) / 2);
            }
            ImGui::SameLine();
            if (ImGui::Button("Nastav na 1600x900")) {
                window_width = 1600;
                window_height = 900;
                glfwSetWindowSize(window, window_width, window_height);
                glfwSetWindowPos(window, (mode->width - window_width) / 2, (mode->height - window_height) / 2);
            }
            ImGui::SameLine();
            if (!fullscreen) {
                if (ImGui::Button("Rezim fullscreen")) {
                    window_width = mode->width;
                    window_height = mode->height;
                    fullscreen = true; // Set fullscreen to true
                }
            } else {
                if (ImGui::Button("Rezim okna")) {
                    window_width = 1280;
                    window_height = 720;
                    glfwSetWindowMonitor(window, nullptr, 0, 0, window_width, window_height, 0);
                    glfwSetWindowPos(window, (mode->width - window_width) / 2, (mode->height - window_height) / 2);
                    fullscreen = false; // Set fullscreen to false
                }
            }

            /* Set the font scale */
            if (ImGui::InputFloat("Velikost fontu", &font_scale, 0.1f, 0.3f, "%.1f")) {
                // On change, clamp the value between 1.0 and 2.0
                if (font_scale < 1.0f) font_scale = 1.0f;
                if (font_scale > 2.0f) font_scale = 2.0f;
            }
            ImGui::SetWindowFontScale(font_scale);

            /* Colors section */
            ImGui::SeparatorText("Barvy");
            static int style_idx = 0; // Overall style
            if (ImGui::Combo("Styl", &style_idx, "Tmavy\0Svetly\0Klasika\0")) {
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
            ImGui::Begin("O aplikaci", &show_about_window,
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse);
            ImGui::SetWindowFontScale(font_scale); // Set the font size
            ImGui::Text("Aplikace byla vytvorena autorem:\nDominik Zappe");
            ImGui::Separator();
            ImGui::Text("Aplikace slouzi jako semestralni prace na predmet IR");
            ImGui::End();
        }
    }
    /* Reset the font */
    ImGui::PopFont();

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
