#pragma once

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <unordered_map>
#include <vector>
#include "cpp_indexer/data/Document.h"

#ifdef __linux__
#define _CRAWLER_PATH_ "crawler.sh"
#define _LANG_DETECTOR_PATH_ "lang_detector.sh"
#define _LANG_DETECTOR_TEXT_PATH_ "lang_detector_text.sh"
#elif _WIN32
#define _CRAWLER_PATH_ "crawler.bat"
#define _LANG_DETECTOR_PATH_ "lang_detector.bat"
#define _LANG_DETECTOR_TEXT_PATH_ "lang_detector_text.bat"
#else

#endif

/**
 * Python script handler class
 * Calls .bat files in /bin directory
 */
class PyHandler {
private:
    /** Paths to the script files */
    constexpr static const char* CRAWLER_PATH = _CRAWLER_PATH_;
    /** Paths to the script files */
    constexpr static const char* LANG_DETECTOR_PATH = _LANG_DETECTOR_PATH_;
    /** Paths to the script files */
    constexpr static const char* LANG_DETECTOR_BAT_PATH = _LANG_DETECTOR_TEXT_PATH_;
    /** Path to the language detector model */
    constexpr static const char* MODEL_PATH = "../src/py_lang_detect/model.bin";

public:
    /**
     * Execute the given command and return the output
     * @param cmd Command to execute
     * @return Output of the command
     */
    static std::string exec(const char* cmd);
    /**
     * Execute the given command and return the output
     * @param cmd Command to execute
     * @return Output of the command
     */
    static std::string exec(const std::string& cmd);

    /**
     * Run the crawler to download the document from the given URL
     * @param url URL to download the document from
     * @return Output of the crawler
     */
    static std::string run_crawler(const std::string& url);
    /**
     * Run the language detector on the given documents
     * @param dir Directory with the text files
     * @return Detected language
     */
    static std::string run_lang_detector(const std::string& dir);
    /**
     * Run the language detector on the given text
     * @param text Text to detect the language of
     * @return Detected language
     */
    static std::string run_lang_detector_text(const std::string& text);

    /**
     * Detect the language of the document
     * @param docs Documents
     * @param verbose Whether to print the progress
     * @return Dictionary of IDs and detected languages
     */
    static std::unordered_map<int, std::string> detect_lang(const std::vector<Document> &docs, bool verbose=true);

    /**
     * Detect the language of the given text
     * @param text Text
     * @param verbose Whether to print the progress
     * @return Detected language
     */
    static std::string detect_lang_text(const std::string &text, bool verbose=true);
};
