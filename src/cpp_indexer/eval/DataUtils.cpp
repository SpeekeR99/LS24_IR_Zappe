#include "DataUtils.h"

std::vector<Document> DataUtils::load_documents(const std::string &filename) {
    /* Read the entire file into a string */
    std::ifstream file(filename);
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    /* Split the string into documents */
    std::vector<std::string> all_tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, '|'))
        all_tokens.push_back(token);

    /* Ignore header, first 4 tokens */
    all_tokens.erase(all_tokens.begin(), all_tokens.begin() + 4);

    std::vector<Document> documents;
    while (all_tokens.size() >= 4) {
        std::string id_str = all_tokens[0];
        id_str.erase(0, 1);
        int id = std::stoi(id_str);
        std::string title = all_tokens[1];
        std::string text = all_tokens[2];
        std::vector<std::string> date_vec = {all_tokens[3]};
        documents.push_back(Document(id, title, date_vec, {}, {}, {}, text));
        all_tokens.erase(all_tokens.begin(), all_tokens.begin() + 4);
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
