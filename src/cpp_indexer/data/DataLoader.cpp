#include "DataLoader.h"

int DataLoader::id_counter = 0;

json DataLoader::load_json(const std::string &path) {
    std::ifstream input(path);
    json data;
    input >> data;
    return data;
}

Document DataLoader::load_json_document(const std::string &path) {
    std::ifstream input(path);
    json data;
    input >> data;
    return {
            DataLoader::id_counter++,
            data["title"][0],
            data["toc"].get<std::vector<std::string>>(),
            data["h1"].get<std::vector<std::string>>(),
            data["h2"].get<std::vector<std::string>>(),
            data["h3"].get<std::vector<std::string>>(),
            data["content"][0]
    };
}

std::vector<json> DataLoader::load_jsons_from_dir(const std::string &path) {
    std::vector<json> jsons;
    for (const auto &entry : std::filesystem::directory_iterator(path))
        if (entry.path().extension() == ".json")
            jsons.push_back(load_json(entry.path().string()));
    return jsons;
}

std::vector<Document> DataLoader::load_json_documents_from_dir(const std::string &path) {
    std::vector<Document> documents;
    for (const auto &entry : std::filesystem::directory_iterator(path))
        if (entry.path().extension() == ".json")
            documents.push_back(load_json_document(entry.path().string()));
    return documents;
}

void DataLoader::save_index_to_file(Indexer &indexer, const string &index_path) {
    std::ofstream output(index_path);
    json data = indexer.to_json();
    output << data.dump(1);
    output.close();
}

void DataLoader::load_index_from_file(Indexer &indexer, const string &index_path) {
    std::ifstream input(index_path);
    json data;
    input >> data;
    indexer.from_json(data);
    input.close();
}
