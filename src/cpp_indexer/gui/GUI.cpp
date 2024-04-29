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
    std::cout << "GLEW version: " << glewGetString(GLEW_VERSION) << std::endl << std::endl;

    /* Setup Platform/Renderer backends */
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    /* Background color */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    /* Load the font */
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, /* Basic Latin + Latin Supplement */
        0x0100, 0x017F, /* Latin Extended-A */
        0x2000, 0x206F, /* General Punctuation */
        0x3000, 0x303F, /* CJK Symbols and Punctuation */
        0,
    };
    ImFontConfig config;
    config.GlyphRanges = ranges;
    this->font = io.Fonts->AddFontFromFileTTF(font_path, font_size, &config);

    /* GREEN */
    ImGuiStyle &style = ImGui::GetStyle();
    for (auto &col : style.Colors)
        col = ImVec4(col.x - 0.04f, col.z + 0.05f, col.y - 0.08f, col.w);

    /* Initialize the index */
    if (FILE_BASED) {
        if (!std::filesystem::exists(FILE_BASED_INDEX_PATH))
            std::filesystem::create_directory(FILE_BASED_INDEX_PATH);
        for (const auto &entry : std::filesystem::directory_iterator(FILE_BASED_INDEX_PATH)) {
            indices.emplace_back(entry.path().filename().replace_extension().string());
            Indexer indexer = Indexer(entry.path().string() + "/");
            indexers.emplace_back(indexer);
        }
    } else {
        if (!std::filesystem::exists(INDEX_PATH))
            std::filesystem::create_directory(INDEX_PATH);
        for (const auto &entry : std::filesystem::directory_iterator(INDEX_PATH)) {
            indices.emplace_back(entry.path().filename().replace_extension().string());
            Indexer indexer = Indexer();
            IndexHandler::load_index(indexer, entry.path().string());
            indexers.emplace_back(indexer);
        }
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
        ImGui::Begin("Hlavní okno", nullptr,
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
            if (ImGui::BeginMenu("Nastavení")) { /* Settings menu */
                if (ImGui::MenuItem("Grafická nastavení...")) {
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
            if (ImGui::BeginTabItem("Vyhledávač")) {
                /* Set up the main GUI configuration window */
                ImGui::Begin("Konfigurace", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2(0, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                if (indices.empty())
                    ImGui::Text("Neexistují žádné indexy!");
                else if (ImGui::BeginCombo("Index", indices[current_index].c_str())) {
                    for (int i = 0; i < indices.size(); i++) {
                        bool is_selected = (current_index == i);
                        if (ImGui::Selectable(indices[i].c_str(), is_selected))
                            current_index = i;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                    if (indexers[current_index].get_max_doc_id())
                        DataLoader::id_counter = indexers[current_index].get_max_doc_id() + 1;
                    else
                        DataLoader::id_counter = 0;
                }

                const char *model_names[] = {"Vektorový", "Booleovský"};
                ImGui::ListBox("Model", &current_model, model_names, IM_ARRAYSIZE(model_names));

                if (current_model == 0) { /* Vector model */
                    const char *field_names[] = {"Vše", "Nadpis", "Obsah"};
                    ImGui::ListBox("Hledat v", &current_field, field_names, IM_ARRAYSIZE(field_names));
                    
                    ImGui::InputInt("K nejlepších", &k_best);
                    if (k_best < 1)
                        k_best = 1;
                }

                ImGui::Checkbox("Detekce jazyka\n(dotazu)", &detect_language);

                if (current_model == 0) {/* Vector model */
                    if (ImGui::Checkbox("Hledání v blízkosti\n(proximity search)", &proximity_search)) {
                        if (proximity_search && phrase_search)
                            phrase_search = false;
                    }

                    if (proximity_search) {
                        ImGui::InputInt("Vzdálenost", &proximity);
                        if (proximity < 1)
                            proximity = 1;
                    }

                    if (ImGui::Checkbox("Hledání fráze\n(phrase search)", &phrase_search)) {
                        if (proximity_search && phrase_search)
                            proximity_search = false;
                    }
                }

                ImGui::End();

                ImGui::Begin("Vyhledávání", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2((float) window_width / 3, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3 * 2, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 80, ImGui::GetCursorPosY() + 20));

                bool find = false;
                if (ImGui::InputTextWithHint("##Dotaz", "Zadejte svůj dotaz...", &query, ImGuiInputTextFlags_EnterReturnsTrue))
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

                    bool do_search = true;
                    if (query.empty()) {
                        std::cout << "Empty query!" << std::endl << std::endl;
                        search_results.clear();
                        do_search = false;
                    }

                    if (do_search && current_model == 0) { /* Vector model */
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
                            std::string snippet;
                            std::vector<int> highlight_index;
                            if (proximity_search)
                                std::tie(snippet, highlight_index) = IndexHandler::create_snippet(index, doc.id, positions, snippet_window_size, proximity);
                            else if (phrase_search)
                                std::tie(snippet, highlight_index) = IndexHandler::create_snippet(index, doc.id, positions, snippet_window_size, 1);
                            else
                                std::tie(snippet, highlight_index) = IndexHandler::create_snippet(index, doc.id, positions, snippet_window_size);
                            result_snippets.emplace_back(snippet);
                            highlight_indices.emplace_back(highlight_index);
                        }
                    } else if (do_search && current_model == 1) { /* Boolean model */
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

                static std::unordered_set<std::string> autocomplete_entries;
                if (!indices.empty())
                    autocomplete_entries = indexers[current_index].get_keywords();

                if (!query.empty()) {
                    std::istringstream iss(query);
                    std::vector<std::string> words((std::istream_iterator<std::string>(iss)),
                                                   std::istream_iterator<std::string>());
                    std::string last_word = words.back();

                    if (ImGui::BeginChild("##ScrollingRegion", ImVec2(0, 100), true)) {
                        for (const auto &autocomplete_entry: autocomplete_entries) {
                            /* Check if the last word is a prefix of the autocomplete entry */
                            /* ignore case */
                            std::string last_word_lower = last_word;
                            std::transform(last_word_lower.begin(), last_word_lower.end(), last_word_lower.begin(), ::tolower);
                            if (autocomplete_entry.compare(0, last_word_lower.size(), last_word_lower) == 0) {
                                if (ImGui::Selectable(autocomplete_entry.c_str())) {
                                    words.pop_back();
                                    words.push_back(autocomplete_entry);
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
                    ImGui::Text("Detekovaný jazyk: %s", query_lang.c_str());

                ImGui::Text("Celkem výsledků: %d", total_results);
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
                            ImGui::Text("Úryvek: ...");
                            ImGui::SameLine();
                            std::istringstream snippet_stream(snippet);
                            std::string word;
                            int index = 0;
                            int size_so_far = 0;
                            int threshold = ImGui::GetCurrentWindow()->Size.x - 400;
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
            if (ImGui::BeginTabItem("Indexování")) {
                /* Set up the Index window */
                ImGui::Begin("Index", nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                /* Set up the main GUI window position, size and font size */
                ImGui::SetWindowPos(ImVec2(0, font_size * 3));
                ImGui::SetWindowSize(ImVec2((float) window_width / 3, (float) window_height));
                ImGui::SetWindowFontScale(font_scale);

                if (indices.empty())
                    ImGui::Text("Neexistují žádné indexy!");
                else if (ImGui::BeginCombo("Vybraný index", indices[current_index].c_str())) {
                    for (int i = 0; i < indices.size(); i++) {
                        bool is_selected = (current_index == i);
                        if (ImGui::Selectable(indices[i].c_str(), is_selected))
                            current_index = i;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                    if (indexers[current_index].get_max_doc_id())
                        DataLoader::id_counter = indexers[current_index].get_max_doc_id() + 1;
                    else
                        DataLoader::id_counter = 0;
                }
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                if (ImGui::Button("Smazat")) {
                    if (FILE_BASED)
                        std::filesystem::remove_all(FILE_BASED_INDEX_PATH + indices[current_index]);
                    else
                        std::filesystem::remove(INDEX_PATH + indices[current_index] + ".json");
                    indices.erase(indices.begin() + current_index);
                    indexers.erase(indexers.begin() + current_index);
                    current_index = 0;
                }
                ImGui::PopStyleColor(3);

                ImGui::InputTextWithHint("Nový index", "Zadejte název indexu...", new_index_name, IM_ARRAYSIZE(new_index_name));
                if (ImGui::Button("Vytvořit")) {
                    indices.emplace_back(new_index_name);
                    if (FILE_BASED) {
                        std::filesystem::create_directory(FILE_BASED_INDEX_PATH + new_index_name);
                        Indexer indexer = Indexer(FILE_BASED_INDEX_PATH + new_index_name + "/", false);
                        indexers.emplace_back(indexer);
                    } else {
                        Indexer indexer = Indexer();
                        indexers.emplace_back(indexer);
                    }
                    current_index = indices.size() - 1;
                    if (indexers[current_index].get_max_doc_id())
                        DataLoader::id_counter = indexers[current_index].get_max_doc_id() + 1;
                    else
                        DataLoader::id_counter = 0;
                }

                ImGui::InputTextWithHint("Data", "Zadejte cestu k datům...", data_path, IM_ARRAYSIZE(data_path));
                if (ImGui::Button("Načíst data")) {
                    auto docs = IndexHandler::load_documents(data_path);
                    auto [tokenized_docs, positions] = IndexHandler::preprocess_documents(docs);
                    if (FILE_BASED) {
                        FileBasedLoader::save_doc_cache(docs, FILE_BASED_INDEX_PATH + indices[current_index] + "/");
                        FileBasedLoader::save_tokenized_docs(tokenized_docs, FILE_BASED_INDEX_PATH + indices[current_index] + "/");
                        FileBasedLoader::save_positions_map(positions, FILE_BASED_INDEX_PATH + indices[current_index] + "/");
                        auto indexer = Indexer(FILE_BASED_INDEX_PATH + indices[current_index] + "/");
                        indexers[current_index] = indexer;
                    } else {
                        auto indexer = Indexer(docs, tokenized_docs, positions);
                        indexers[current_index] = indexer;
                        IndexHandler::save_index(indexer, "../index/" + indices[current_index] + ".json");
                    }
                }

                ImGui::InputInt("ID", &current_doc_id);
                if (ImGui::Button("Načíst dokument")) {
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

                ImGui::InputTextWithHint("URL", "Zadejte url z witcher.fandom.com/cs", url, IM_ARRAYSIZE(url));
                if (ImGui::Button("Stáhnout dokument z URL")) {
                    IndexHandler::add_doc_url(indexers[current_index], url);
                    if (!FILE_BASED)
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

                if (ImGui::Button("Vytvořit")) {
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
                    if (!FILE_BASED)
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
                    if (!FILE_BASED)
                        IndexHandler::save_index(indexers[current_index], "../index/" + indices[current_index] + ".json");
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                if (ImGui::Button("Smazat")) {
                    std::vector<int> id_vec = {current_doc_id};
                    IndexHandler::remove_docs(indexers[current_index], id_vec);
                    if (!FILE_BASED)
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
            ImGui::Begin("Grafická nastavení", &show_settings_window,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            /* Set the window size */
            ImGui::SetWindowSize(ImVec2(600, 400));

            /* Window settings section */
            ImGui::SeparatorText("Nastavení okna");

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
                if (ImGui::Button("Režim fullscreen")) {
                    window_width = mode->width;
                    window_height = mode->height;
                    fullscreen = true; // Set fullscreen to true
                }
            } else {
                if (ImGui::Button("Režim okna")) {
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
            if (ImGui::Combo("Styl", &style_idx, "Tmavý\0Světlý\0Klasika\0")) {
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
            ImGui::SetNextWindowFocus();
            ImGui::Begin("O aplikaci", &show_about_window,
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse);
            ImGui::SetWindowFontScale(font_scale); // Set the font size
            ImGui::Text("Aplikace byla vytvořena autorem:\nDominik Zappe");
            ImGui::Separator();
            ImGui::Text("Aplikace slouží jako semestrální práce na předmet IR");
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
