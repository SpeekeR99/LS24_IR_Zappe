#pragma once

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>

/**
 * Python script handler class
 * Calls .bat files in /bin directory
 */
class PyHandler {
private:
    /** Paths to the .bat files */
    constexpr static const char* CRAWLER_BAT = "crawler.bat";
    /** Paths to the .bat files */
    constexpr static const char* LANG_DETECTOR_BAT = "lang_detector.bat";
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
     * Run the language detector on the given text
     * @param text Text to detect the language of
     * @return Detected language
     */
    static std::string run_lang_detector(const std::string& text);
};