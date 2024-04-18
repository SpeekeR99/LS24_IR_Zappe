#pragma once

#include <fstream>
#include <iostream>
#include "Document.h"

struct Query {
    std::string id;
    std::string title;
    std::string description;
};

class DataUtils {
public:
    static std::vector<Document> load_documents(const std::string &filename="../src/cpp_indexer/eval/data/czechData.csv");
    static std::vector<Query> load_queries(const std::string &filename="../src/cpp_indexer/eval/data/czechQueries.csv");
};
