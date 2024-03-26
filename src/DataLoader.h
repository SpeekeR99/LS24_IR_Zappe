#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include <nlohmann/json.hpp>
#include "Document.h"

using json = nlohmann::json;

/**
 * Class for loading data from JSON files
 */
class DataLoader {
public:
    /**
     * Load JSON from the given path
     * @param path Filepath to the JSON file
     * @return json object
     */
    static json load_json(const std::string &path);
    /**
     * Load JSON document from the given path
     * @param path Filepath to the JSON file
     * @return Document object
     */
    static Document load_json_document(const std::string &path);
    /**
     * Load JSONs from the given directory
     * @param path Directory path with JSON files
     * @return Vector of json objects
     */
    static std::vector<json> load_jsons_from_dir(const std::string &path);
    /**
     * Load JSON documents from the given directory
     * @param path Directory path with JSON files
     * @return Vector of Document objects
     */
    static std::vector<Document> load_json_documents_from_dir(const std::string &path);
};
