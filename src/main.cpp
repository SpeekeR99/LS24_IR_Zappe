#include <iostream>
#include <chrono>
#include "DataLoader.h"
#include "Preprocessor.h"
#include "Indexer.h"

/**
 * Print the search results for the given query
 * @param query Query
 * @param result Result
 * @param indexer Indexer
 * @param doc_cache Document cache
 */
void print_query_results(const std::string &query, const std::pair<std::vector<int>, std::vector<float>> &result, Indexer &indexer, const std::unordered_map<int, Document> &doc_cache) {
    std::cout << "Top " << result.first.size() << " search results for \"" << query << "\":" << std::endl;
    for (auto i = 0; i < result.first.size(); i++) {
        auto doc_id = result.first[i];
        auto score = result.second[i];
        auto doc = indexer.get_doc(doc_id);
        std::cout << "ID: " << doc.id << ", Title: " << doc_cache.at(doc.id).title << std::endl << "(Score: " << score << ")" << std::endl;
    }
    std::cout << std::endl;
}

/**
 * Main function
 * @return Exit code
 */
int main() {
    /* Load documents */
    std::cout << "Loading documents..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto docs = DataLoader::load_json_documents_from_dir("../data");
    std::unordered_map<int, Document> doc_cache;
    for (auto &doc : docs)
        doc_cache.insert({doc.id, doc});
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Loaded " << docs.size() << " documents" << std::endl;
    std::cout << "Loading done in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    /* Preprocess documents */
    std::cout << "Preprocessing documents..." << std::endl;
    auto preprocessor = Preprocessor();
    auto tokenized_docs = std::vector<TokenizedDocument>();
    start = std::chrono::high_resolution_clock::now();
    for (auto &doc : docs)
        tokenized_docs.emplace_back(
            doc.id,
            preprocessor.preprocess_text(doc.title, true, false),
            preprocessor.preprocess_text(doc.toc, true, false),
            preprocessor.preprocess_text(doc.h1, true, false),
            preprocessor.preprocess_text(doc.h2, true, false),
            preprocessor.preprocess_text(doc.h3, true, false),
            preprocessor.preprocess_text(doc.content, true, true)
        );
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Preprocessed " << tokenized_docs.size() << " documents" << std::endl;
    std::cout << "Preprocessing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    /* Index documents */
    std::cout << "Indexing documents..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    auto indexer = Indexer(tokenized_docs);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Indexed " << indexer.get_collection_size() << " documents and " << indexer.get_index_size() << " words using TF-IDF" << std::endl;
    std::cout << "(Indexed " << indexer.get_title_index_size() << " words in titles using TF-IDF)" << std::endl;
    std::cout << "Indexing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    /* Search "Geralt z Rivie" */
    std::string query = "Geralt z Rivie";
    auto query_tokens = preprocessor.preprocess_text(query, true, false);
    auto result = indexer.search(query_tokens, 3);
    print_query_results(query, result, indexer, doc_cache);

    /* Update the document, so it contains "Geralt z Rivie" more and so it is way more relevant (see Score print) */
    std::cout << "Updating document with ID 550..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    indexer.update_doc(550, TokenizedDocument(550, {"uz", "ne", "geralt", "z", "rivie"}, {}, {}, {}, {}, {"geralt", "z", "rivie", "geralt", "z", "rivie", "geralt", "z", "rivie", "geralt", "z", "rivie"}));
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Document updated and everything reindexed in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    /* Search "Geralt z Rivie" again */
    result = indexer.search(query_tokens, 3);
    print_query_results(query, result, indexer, doc_cache);

    /* Remove the document completely now and see how the score of others changes too, because overall IDF changes */
    std::cout << "Removing document with ID 550..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    indexer.remove_doc(550);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Document removed and everything reindexed in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    /* Search "Geralt z Rivie" again and see how the score of others changes (also ID 550 is gone, shocking!) */
    result = indexer.search(query_tokens, 10);
    print_query_results(query, result, indexer, doc_cache);

    /* Search "Geralt z Rivie" boolean */
    query = "NOT Geralt OR (z AND NOT NOT Rivie)";
    auto bool_tokens = preprocessor.parse_bool_query(query);

    std::cout << query << std::endl;

    for (auto &token : bool_tokens)
        std::cout << token << " ";
    std::cout << std::endl;

//    auto result_ids = indexer.search(bool_tokens, bool_ops);
//    std::cout << "Documents that contain \"" << query << "\":" << std::endl;
//    std::cout << "Found " << result_ids.size() << " documents" << std::endl << "Results:" << std::endl;
//    for (auto &doc_id : result_ids)
//        std::cout << "ID: " << doc_id << ", Title: " << doc_cache.at(doc_id).title << std::endl;
//    std::cout << std::endl;

    return EXIT_SUCCESS;
}
