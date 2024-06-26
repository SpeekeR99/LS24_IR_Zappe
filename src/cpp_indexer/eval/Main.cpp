#include <chrono>
#include "DataUtils.h"
#include "IndexHandler.h"

/** File based index */
bool FILE_BASED = false;
/** Language detection is REALLY sloooow */
bool DETECT_LANG = false;
/** Use lemmatization or stemming */
bool USE_LEMMA = true;

int main() {
    std::cout << "Loading documents..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Document> docs = DataUtils::load_documents();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Loaded " << docs.size() << " documents" << std::endl;
    std::cout << "Documents loaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    auto [tokenized_docs, positions_map] = IndexHandler::preprocess_documents(docs);

    std::cout << "Loading queries..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::vector<Query> queries = DataUtils::load_queries();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Queries loaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    auto indexer = Indexer(docs, tokenized_docs, positions_map);
//    auto indexer = Indexer();
//    IndexHandler::load_index(indexer, "../src/cpp_indexer/eval/data/index.json");

//    IndexHandler::save_index(indexer, "../src/cpp_indexer/eval/data/index.json");

    std::cout << "Testing queries..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::ofstream output("../src/cpp_indexer/eval/data/results_" + std::to_string(std::chrono::system_clock::to_time_t(start)) + ".txt");
    for (const Query &query : queries) {
//        std::string query_str = query.title + " " + query.description;
        std::string query_str = query.title;
//        std::string query_str = query.description;
        std::cout << "Query " << query.id << ": " << query_str << std::endl;
        auto [docs_result, scores, positions] = IndexHandler::search(indexer, query_str, 1000000, FieldType::ALL, 0, false);
        std::cout << "Writing results..." << std::endl;
        for (auto i = 0; i < docs_result.size(); i++) {
            std::string line = query.id + " Q0 d" + std::to_string(docs_result[i].id) + " " + std::to_string(i + 1) + " " + std::to_string(scores[i]) + " runindex1";
//            std::string line = query.id + " Q0 d" + std::to_string(docs_result[i].id) + " " + std::to_string(i + 1) + " " + std::to_string(scores[i] / 2.5) + " runindex1";
            output << line << std::endl;
        }
    }
    output.close();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Queries tested in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    return EXIT_SUCCESS;
}