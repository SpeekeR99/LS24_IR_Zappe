#include "DataUtils.h"

std::vector<Document> DataUtils::load_documents(const std::string &filename) {
    std::ifstream file(filename);

    std::vector<Document> documents;
    std::string line;

    bool first_line = true;

    while (std::getline(file, line)) {
        if (first_line) {
            first_line = false;
            continue;
        }

        std::vector<std::string> tokens;
        std::string token;
        std::istringstream token_stream(line);
        while (std::getline(token_stream, token, '|'))
            tokens.push_back(token);
        std::string id_str = tokens[0];
        id_str.erase(0, 1);
        int id = std::stoi(id_str);
        std::string title = tokens[1];
        std::string text = tokens[2];
        std::vector<std::string> date_vec = {tokens[3]};
        documents.push_back(Document(id, title, date_vec, {}, {}, {}, text));
    }

    file.close();
    return documents;
}

std::vector<Query> DataUtils::load_queries(const std::string &filename) {
    std::ifstream file(filename);

    std::vector<Query> queries;
    std::string line;

    bool first_line = true;

    while (std::getline(file, line)) {
        if (first_line) {
            first_line = false;
            continue;
        }
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream token_stream(line);
        while (std::getline(token_stream, token, '|'))
            tokens.push_back(token);
        queries.push_back(Query{tokens[0], tokens[1], tokens[2]});
    }

    file.close();
    return queries;
}
