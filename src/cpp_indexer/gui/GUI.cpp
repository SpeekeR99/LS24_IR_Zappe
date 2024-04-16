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
    ImGui::StyleColorsClassic();

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

    /* GREEN */
    ImGuiStyle &style = ImGui::GetStyle();
    for (auto &col : style.Colors)
        col = ImVec4(col.x - 0.04f, col.z + 0.05f, col.y - 0.08f, col.w);

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

                if (ImGui::Checkbox("Hledadni v blizkosti\n(proximity search)", &proximity_search)) {
                    if (proximity_search && phrase_search)
                        phrase_search = false;
                }

                if (proximity_search) {
                    ImGui::InputInt("Vzdalenost", &proximity);
                    if (proximity < 1)
                        proximity = 1;
                }

                if (ImGui::Checkbox("Hledani fráze\n(phrase search)", &phrase_search)) {
                    if (proximity_search && phrase_search)
                        proximity_search = false;
                }

                ImGui::End();

                ImGui::Begin("Vyhledavani", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2((float) window_width / 3, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3 * 2, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 80, ImGui::GetCursorPosY() + 20));

                bool find = false;
                if (ImGui::InputText("##Dotaz", &query, ImGuiInputTextFlags_EnterReturnsTrue))
                    find = true;
                ImGui::SameLine();
                if (ImGui::Button("Hledej") || find) {
                    if (detect_language)
                        query_lang = PyHandler::detect_lang_text(query);

                    auto index = indexers[current_index];
                    FieldType field = FieldType::ALL;
                    if (current_field == 1)
                        field = FieldType::TITLE;
                    else if (current_field == 2)
                        field = FieldType::CONTENT;

                    result_snippets.clear();
                    highlight_indices.clear();

                    if (current_model == 0) { /* Vector model */
                        std::vector<Document> result;
                        std::vector<float> scores;
                        std::map<std::string, std::map<int, std::vector<int>>> positions;
                        if (proximity_search)
                            std::tie(result, scores, positions) = IndexHandler::search(index, query, k_best, field, proximity);
                        else if (phrase_search)
                            std::tie(result, scores, positions) = IndexHandler::search(index, query, k_best, field, 1);
                        else
                            std::tie(result, scores, positions) = IndexHandler::search(index, query, k_best, field);
                        this->search_results = result;
                        for (const auto &doc : search_results) {
                            auto [snippet, highlight_index] = IndexHandler::create_snippet(index, doc.id, positions, snippet_window_size);
                            result_snippets.emplace_back(snippet);
                            highlight_indices.emplace_back(highlight_index);
                        }
                    } else { /* Boolean model */
                        auto [result, positions] = IndexHandler::search(index, query, field);
                        this->search_results = result;
                        for (const auto &doc : search_results) {
                            auto [snippet, highlight_index] = IndexHandler::create_snippet(index, doc.id, positions, snippet_window_size);
                            result_snippets.emplace_back(snippet);
                            highlight_indices.emplace_back(highlight_index);
                        }
                    }
                    this->total_results = this->search_results.size();
                }

                static auto autocomplete_entries = indexers[current_index].get_keywords();

                if (!query.empty()) {
                    std::istringstream iss(query);
                    std::vector<std::string> words((std::istream_iterator<std::string>(iss)),
                                                   std::istream_iterator<std::string>());
                    std::string lastWord = words.back();

                    if (ImGui::BeginChild("##ScrollingRegion", ImVec2(0, 100), true)) {
                        for (const auto &autocompleteEntry: autocomplete_entries) {
                            /* Check if the last word is a prefix of the autocomplete entry */
                            if (autocompleteEntry.compare(0, lastWord.size(), lastWord) == 0) {
                                if (ImGui::Selectable(autocompleteEntry.c_str())) {
                                    words.pop_back();
                                    words.push_back(autocompleteEntry);
                                    query = "";
                                    for (const auto &word: words)
                                        query += word + " ";
                                }
                            }
                        }
                    }
                    ImGui::EndChild();
                }

                if (detect_language)
                    ImGui::Text("Detekovany jazyk: %s", query_lang.c_str());

                ImGui::Text("Celkem vysledku: %d", total_results);
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Výsledky")) {
                    for (auto i = 0; i < total_results; i++) {
                        auto doc = search_results[i];
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode(("Dokument " + std::to_string(doc.id)).c_str())) {
                            ImGui::Text("Nadpis: %s", doc.title.c_str());
                            ImGui::Text("Jazyk: %s", doc.lang.c_str());
                            auto snippet = result_snippets[i];
                            auto highlight_index = highlight_indices[i];

                            /* Iterate word by word to highlight the words */
                            ImGui::Text("Uryvek: ...");
                            ImGui::SameLine();
                            std::istringstream snippet_stream(snippet);
                            std::string word;
                            int index = 0;
                            int size_so_far = 0;
                            while (std::getline(snippet_stream, word, ' ')) {
                                auto size = ImGui::CalcTextSize(word.c_str());
                                if (std::find(highlight_index.begin(), highlight_index.end(), index++) != highlight_index.end()) {
                                    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetCursorScreenPos(),
                                                                             ImVec2(ImGui::GetCursorScreenPos().x + size.x,
                                                                                    ImGui::GetCursorScreenPos().y + size.y),
                                                                             IM_COL32(20, 200, 20, 128));
                                }
                                ImGui::Text("%s", word.c_str());
                                size_so_far += size.x;
                                int threshold = ImGui::GetCurrentWindow()->Size.x - 400;
                                if (size_so_far < threshold)
                                    ImGui::SameLine();
                                else
                                    size_so_far = 0;
                            }
                            ImGui::Text("...");

                            ImGui::Separator();
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
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
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                if (ImGui::Button("Smazat")) {
                    std::filesystem::remove("../index/" + indices[current_index] + ".json");
                    indices.erase(indices.begin() + current_index);
                    indexers.erase(indexers.begin() + current_index);
                    current_index = 0;
                }
                ImGui::PopStyleColor(3);

                ImGui::InputText("Novy index", new_index_name, IM_ARRAYSIZE(new_index_name));
                if (ImGui::Button("Vytvorit")) {
                    indices.emplace_back(new_index_name);
                    Indexer indexer = Indexer();
                    indexers.emplace_back(indexer);
                    current_index = indices.size() - 1;
                }

                ImGui::InputText("Data", data_path, IM_ARRAYSIZE(data_path));
                if (ImGui::Button("Nacist data")) {
                    auto docs = IndexHandler::load_documents(data_path);
                    auto [tokenized_docs, positions] = IndexHandler::preprocess_documents(docs);
                    auto indexer = Indexer(docs, tokenized_docs, positions);
                    indexers[current_index] = indexer;
                    IndexHandler::save_index(indexer, "../index/" + indices[current_index] + ".json");
                }

                ImGui::InputInt("ID", &current_doc_id);
                if (ImGui::Button("Nacist dokument")) {
                    std::vector<int> id_vec = {current_doc_id};
                    auto doc = IndexHandler::get_docs(indexers[current_index], id_vec)[0];
                    current_doc_title = doc.title;
                    current_doc_toc = "";
                    for (const auto &token : doc.toc)
                        current_doc_toc += token + "\n";
                    current_doc_h1 = "";
                    for (const auto &token : doc.h1)
                        current_doc_h1 += token + "\n";
                    current_doc_h2 = "";
                    for (const auto &token : doc.h2)
                        current_doc_h2 += token + "\n";
                    current_doc_h3 = "";
                    for (const auto &token : doc.h3)
                        current_doc_h3 += token + "\n";
                    current_doc_content = doc.content;
                }

                ImGui::InputText("URL", url, IM_ARRAYSIZE(url));
                if (ImGui::Button("Stahnout dokument z URL")) {
                    IndexHandler::add_doc_url(indexers[current_index], url);
                    IndexHandler::save_index(indexers[current_index], "../index/" + indices[current_index] + ".json");
                }

                ImGui::End();

                /* Set up the Document window */
                ImGui::Begin("Dokument", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2((float) window_width / 3, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3 * 2, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                ImGui::InputText("Nadpis", &current_doc_title);
                ImGui::InputTextMultiline("Osnova", &current_doc_toc, ImVec2(0, font_size * 4));
                ImGui::InputTextMultiline("H1", &current_doc_h1, ImVec2(0, font_size * 4));
                ImGui::InputTextMultiline("H2", &current_doc_h2, ImVec2(0, font_size * 4));
                ImGui::InputTextMultiline("H3", &current_doc_h3, ImVec2(0, font_size * 4));
                ImGui::InputTextMultiline("Obsah", &current_doc_content);

                if (ImGui::Button("Vytvorit")) {
                    std::vector<std::string> toc;
                    std::istringstream toc_stream(current_doc_toc);
                    std::string toc_line;
                    while (std::getline(toc_stream, toc_line))
                        toc.emplace_back(toc_line);

                    std::vector<std::string> h1;
                    std::istringstream h1_stream(current_doc_h1);
                    std::string h1_line;
                    while (std::getline(h1_stream, h1_line))
                        h1.emplace_back(h1_line);

                    std::vector<std::string> h2;
                    std::istringstream h2_stream(current_doc_h2);
                    std::string h2_line;
                    while (std::getline(h2_stream, h2_line))
                        h2.emplace_back(h2_line);

                    std::vector<std::string> h3;
                    std::istringstream h3_stream(current_doc_h3);
                    std::string h3_line;
                    while (std::getline(h3_stream, h3_line))
                        h3.emplace_back(h3_line);

                    int id = indexers[current_index].get_collection_size();

                    Document doc = {id, current_doc_title, toc, h1, h2, h3, current_doc_content};
                    std::vector<int> id_vec = {doc.id};
                    std::vector<Document> doc_vec = {doc};

                    IndexHandler::add_docs(indexers[current_index], doc_vec);
                    IndexHandler::save_index(indexers[current_index], "../index/" + indices[current_index] + ".json");
                }
                ImGui::SameLine();
                if (ImGui::Button("Aktualizovat")) {
                    std::vector<std::string> toc;
                    std::istringstream toc_stream(current_doc_toc);
                    std::string toc_line;
                    while (std::getline(toc_stream, toc_line))
                        toc.emplace_back(toc_line);

                    std::vector<std::string> h1;
                    std::istringstream h1_stream(current_doc_h1);
                    std::string h1_line;
                    while (std::getline(h1_stream, h1_line))
                        h1.emplace_back(h1_line);

                    std::vector<std::string> h2;
                    std::istringstream h2_stream(current_doc_h2);
                    std::string h2_line;
                    while (std::getline(h2_stream, h2_line))
                        h2.emplace_back(h2_line);

                    std::vector<std::string> h3;
                    std::istringstream h3_stream(current_doc_h3);
                    std::string h3_line;
                    while (std::getline(h3_stream, h3_line))
                        h3.emplace_back(h3_line);

                    Document doc = {current_doc_id, current_doc_title, toc, h1, h2, h3, current_doc_content};
                    std::vector<int> id_vec = {doc.id};
                    std::vector<Document> doc_vec = {doc};

                    IndexHandler::update_docs(indexers[current_index], id_vec, doc_vec);
                    IndexHandler::save_index(indexers[current_index], "../index/" + indices[current_index] + ".json");
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                if (ImGui::Button("Smazat")) {
                    std::vector<int> id_vec = {current_doc_id};
                    IndexHandler::remove_docs(indexers[current_index], id_vec);
                    IndexHandler::save_index(indexers[current_index], "../index/" + indices[current_index] + ".json");
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
            static int style_idx = 2; // Overall style
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
                /* GREEN */
                ImGuiStyle &style = ImGui::GetStyle();
                for (auto &col : style.Colors)
                    col = ImVec4(col.x - 0.04f, col.z + 0.05f, col.y - 0.08f, col.w);
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
