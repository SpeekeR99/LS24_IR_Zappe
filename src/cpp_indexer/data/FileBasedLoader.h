#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include "Document.h"
#include "TF_IDF.h"

using json = nlohmann::json;

struct map_element;

/**
 * Class for loading and saving the file based index files
 */
class FileBasedLoader {
public:
    static void save_doc_cache(const std::vector<Document> &docs, const std::string &index_path_dir);
    static void save_doc_cache(const std::unordered_map<int, Document> &doc_cache, const std::string &index_path_dir);
    static std::unordered_map<int, Document> load_doc_cache(const std::string &index_path_dir);

    static void save_tokenized_docs(const std::vector<TokenizedDocument> &docs, const std::string &index_path_dir);
    static std::vector<TokenizedDocument> load_tokenized_docs(const std::string &index_path_dir);

    static void save_positions_map(const std::map<std::string, std::map<int, std::vector<int>>> &positions_map, const std::string &index_path_dir);
    static std::map<std::string, std::map<int, std::vector<int>>> load_positions_map(const std::string &index_path_dir);

    static void save_tf_idf(const std::map<int, std::map<std::string, float>> &tf_idf_docs, std::map<std::string, float> &idf, const std::string &index_path_dir, bool title = false);
    static std::map<std::string, map_element> load_tf_idf(const std::string &index_path_dir, bool title = false);

    static void save_tf_idf_norms(const std::map<int, std::map<std::string, float>> &tf_idf_docs, const std::string &index_path_dir, bool title = false);
    static std::map<int, float> load_tf_idf_norms(const std::string &index_path_dir, bool title = false);
};
