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
    auto cmd = std::string(CRAWLER_BAT) + " " + url;
    return exec(cmd);
}

std::string PyHandler::run_lang_detector(const std::string &dir) {
    auto cmd = std::string(LANG_DETECTOR_BAT) + " " + MODEL_PATH + " " + dir;
    return exec(cmd);
}
