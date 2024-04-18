#include "FileBasedLoader.h"

void FileBasedLoader::save_doc_cache(const std::vector<Document> &docs, const std::string &index_path_dir) {
    json j;
    for (const auto &doc : docs)
        j[std::to_string(doc.id)] = doc.to_json();
    std::ofstream output(index_path_dir + "doc_cache.json");
    output << j.dump(1);
    output.close();
}

void FileBasedLoader::save_doc_cache(const std::unordered_map<int, Document> &doc_cache, const std::string &index_path_dir) {
    json j;
    for (const auto &[id, doc] : doc_cache)
        j[std::to_string(id)] = doc.to_json();
    std::ofstream output(index_path_dir + "doc_cache.json");
    output << j.dump(1);
    output.close();
}

std::unordered_map<int, Document> FileBasedLoader::load_doc_cache(const std::string &index_path_dir) {
    std::ifstream input(index_path_dir + "doc_cache.json");
    json j;
    input >> j;
    input.close();
    std::unordered_map<int, Document> doc_cache;
    for (const auto &item : j.items()) {
        Document temp = Document();
        temp.from_json(item.value());
        doc_cache[std::stoi(item.key())] = temp;
    }
    return doc_cache;
}

void FileBasedLoader::save_tokenized_docs(const std::vector<TokenizedDocument> &docs, const std::string &index_path_dir) {
    json j;
    for (const auto &doc : docs)
        j[std::to_string(doc.id)] = doc.to_json();
    std::ofstream output(index_path_dir + "tokenized_docs.json");
    output << j.dump(1);
    output.close();
}

std::vector<TokenizedDocument> FileBasedLoader::load_tokenized_docs(const std::string &index_path_dir) {
    std::ifstream input(index_path_dir + "tokenized_docs.json");
    json j;
    input >> j;
    input.close();
    std::vector<TokenizedDocument> docs;
    for (const auto &item : j.items()) {
        TokenizedDocument temp = TokenizedDocument();
        temp.from_json(item.value());
        docs.push_back(temp);
    }
    return docs;
}

void FileBasedLoader::save_positions_map(const std::map<std::string, std::map<int, std::vector<int>>> &positions_map, const std::string &index_path_dir) {
    json j;
    for (const auto &item : positions_map) {
        j[item.first] = json::object();
        for (const auto &item2 : item.second) {
            j[item.first][std::to_string(item2.first)] = json::array();
            for (const auto &pos : item2.second)
                j[item.first][std::to_string(item2.first)].push_back(pos);
        }
    }
    std::ofstream output(index_path_dir + "positions_map.json");
    output << j.dump(1);
    output.close();
}

std::map<std::string, std::map<int, std::vector<int>>> FileBasedLoader::load_positions_map(const std::string &index_path_dir) {
    std::ifstream input(index_path_dir + "positions_map.json");
    json j;
    input >> j;
    input.close();
    std::map<std::string, std::map<int, std::vector<int>>> positions_map;
    for (const auto &item : j.items()) {
        std::map<int, std::vector<int>> temp;
        for (const auto &item2 : item.value().items()) {
            std::vector<int> temp2;
            for (const auto &pos : item2.value())
                temp2.push_back(pos);
            temp[std::stoi(item2.key())] = temp2;
        }
        positions_map[item.key()] = temp;
    }
    return positions_map;
}

void FileBasedLoader::save_tf_idf(const std::map<int, std::map<std::string, float>> &tf_idf_docs, std::map<std::string, float> &idf, const std::string &index_path_dir, bool title) {
    json j;
    /* save in format word -> (doc_id -> (tf-idf), idf) */
    for (const auto &[doc_id, doc_tf_idf]: tf_idf_docs) {
        for (const auto &[word, value]: doc_tf_idf) {
            j[word]["doc_tf_idf"].push_back({doc_id, value});
            j[word]["idf"] = idf[word];
        }
    }
    std::ofstream output;
    if (title)
        output.open(index_path_dir + "title_tf_idf.json");
    else
        output.open(index_path_dir + "tf_idf.json");
    output << j.dump(1);
    output.close();
}

std::map<std::string, map_element> FileBasedLoader::load_tf_idf(const std::string &index_path_dir, bool title) {
    std::ifstream input;
    if (title)
        input.open(index_path_dir + "title_tf_idf.json");
    else
        input.open(index_path_dir + "tf_idf.json");
    json j;
    input >> j;
    input.close();
    std::map<std::string, map_element> map_ele;
    for (const auto &item : j.items()) {
        map_element element;
        element.idf = item.value()["idf"];
        for (const auto &pair : item.value()["doc_tf_idf"])
            element.doc_tf_idf.emplace_back(pair[0], pair[1]);
        map_ele[item.key()] = element;
    }
    return map_ele;
}

void FileBasedLoader::save_tf_idf_norms(const std::map<int, std::map<std::string, float>> &tf_idf_docs, const std::string &index_path_dir, bool title) {
    json j;
    for (const auto &[doc_id, doc_tf_idf] : tf_idf_docs) {
        float norm = 0;
        for (const auto &[word, value] : doc_tf_idf)
            norm += value * value;
        j[std::to_string(doc_id)] = std::sqrt(norm);
    }
    std::ofstream output;
    if (title)
        output.open(index_path_dir + "title_norms.json");
    else
        output.open(index_path_dir + "norms.json");
    output << j.dump(1);
    output.close();
}

std::map<int, float> FileBasedLoader::load_tf_idf_norms(const std::string &index_path_dir, bool title) {
    std::ifstream input;
    if (title)
        input.open(index_path_dir + "title_norms.json");
    else
        input.open(index_path_dir + "norms.json");
    json j;
    input >> j;
    input.close();
    std::map<int, float> norms;
    for (const auto &item : j.items())
        norms[std::stoi(item.key())] = item.value();
    return norms;
}


