#include "PyHandler.h"

std::string PyHandler::exec(const char *cmd) {
    std::array<char, 256> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

    if (!pipe)
        throw std::runtime_error("[ERROR]: popen() failed!");

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
        result += buffer.data();

    return result;
}

std::string PyHandler::exec(const std::string &cmd) {
    return exec(cmd.c_str());
}

std::string PyHandler::run_crawler(const std::string &url) {
    auto cmd = std::string(CRAWLER_PATH) + " " + url;
    return exec(cmd);
}

std::string PyHandler::run_lang_detector(const std::string &dir) {
    auto cmd = std::string(LANG_DETECTOR_PATH) + " " + MODEL_PATH + " " + dir;
    return exec(cmd);
}

std::string PyHandler::run_lang_detector_text(const std::string &text) {
    auto cmd = std::string(LANG_DETECTOR_BAT_PATH) + " " + MODEL_PATH + " " + text;
    return exec(cmd);
}

std::unordered_map<int, std::string> PyHandler::detect_lang(const std::vector<Document> &docs, bool verbose) {
    if (verbose)
        std::cout << "Detecting language of " << docs.size() << " documents..." << std::endl;

    auto t_start = std::chrono::high_resolution_clock::now();
    /* mkdir tmp_detect */
    std::filesystem::create_directory("tmp_detect");

    /* Concatenate the title and content */
    for (auto &doc : docs) {
        std::ofstream file("tmp_detect/" + std::to_string(doc.id) + ".txt");
        file << doc.title << " " << doc.content;
        file.close();
    }

    /* Detect the language */
    auto result = PyHandler::run_lang_detector("tmp_detect/");

    /* Remove the tmp_detect directory */
    std::filesystem::remove_all("tmp_detect");

    /* Find the language in result based on print(f"Predicted language: {labels}") */
    std::string start_str = "Predicted languages: ";
    auto start = result.find(start_str);
    auto lang = result.substr(start + start_str.size());

    auto t_end = std::chrono::high_resolution_clock::now();

    if (verbose)
        std::cout << "Detected languages: " << lang << "Detection done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;

    /* lang = {'0': 'cs', '1': 'en'} */
    auto langs = std::unordered_map<int, std::string>();
    auto start_idx = 0;
    while (true) {
        auto start = lang.find('\'', start_idx);
        if (start == std::string::npos)
            break;
        auto end = lang.find('\'', start + 1);
        auto id = std::stoi(lang.substr(start + 1, end - start - 1));
        start = lang.find('\'', end + 1);
        end = lang.find('\'', start + 1);
        auto lang_str = lang.substr(start + 1, end - start - 1);
        langs[id] = lang_str;
        start_idx = end + 1;

        langs.insert({id, lang_str});
    }

    return langs;
}

std::string PyHandler::detect_lang_text(const std::string &text, bool verbose) {
    if (verbose)
        std::cout << "Detecting language of: " << text << std::endl;

    auto t_start = std::chrono::high_resolution_clock::now();

    /* Detect the language */
    auto result = PyHandler::run_lang_detector_text(text);

    /* Find the language in result based on print(f"Predicted language: {label}") */
    std::string start_str = "Predicted language: ";
    auto start = result.find(start_str);
    auto lang = result.substr(start + start_str.size());

    auto t_end = std::chrono::high_resolution_clock::now();

    if (verbose)
        std::cout << "Detected language: " << lang << "Detection done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;

    return lang;
}
