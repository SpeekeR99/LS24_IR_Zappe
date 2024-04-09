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
void print_query_results(const std::string &query, const std::pair<std::vector<int>, std::vector<float>> &result, Indexer indexer, const std::unordered_map<int, Document> &doc_cache) {
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
 * Load documents from the given directory
 * @param dir_path Directory path
 * @return Documents as a vector and as a map with IDs (cache)
 */
std::pair<std::vector<Document>, std::unordered_map<int, Document>> load_documents(const std::string &dir_path) {
    std::cout << "Loading documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();
    
    auto docs = DataLoader::load_json_documents_from_dir("../data");
    std::unordered_map<int, Document> doc_cache;
    for (auto &doc : docs)
        doc_cache.insert({doc.id, doc});
    
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "Loaded " << docs.size() << " documents" << std::endl;
    std::cout << "Loading done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;

    return {docs, doc_cache};
}

/**
 * Preprocess documents (Documents -> TokenizedDocuments)
 * @param preprocessor Preprocessor object instance
 * @param docs Documents to preprocess
 * @return Tokenized documents (preprocessed)
 */
std::vector<TokenizedDocument> preprocess_documents(Preprocessor &preprocessor, std::vector<Document> &docs) {
    std::cout << "Preprocessing documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();
    
    auto tokenized_docs = std::vector<TokenizedDocument>();
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
    
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "Preprocessed " << tokenized_docs.size() << " documents" << std::endl;
    std::cout << "Preprocessing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;

    return tokenized_docs;
}

/**
 * Add documents to the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param preprocessor Preprocessor object instance
 * @param docs Documents to add
 */
void add_docs(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, Preprocessor &preprocessor, std::vector<Document> &docs) {
    auto &indexer = indexer_cache_pair.first;
    auto &doc_cache = indexer_cache_pair.second;

    auto tokenized_docs = preprocess_documents(preprocessor, docs);

    indexer.add_docs(tokenized_docs);

    /* Update the cache */
    for (auto &doc : docs)
        doc_cache.insert({doc.id, doc});
}

/**
 * Get documents from the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param doc_ids Document IDs
 * @return Documents
 */
std::vector<Document> get_docs(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, std::vector<int> &doc_ids) {
    std::vector<Document> result;
    result.reserve(doc_ids.size());

    for (const auto &doc_id : doc_ids) {
        try {
            result.emplace_back(indexer_cache_pair.second.at(doc_id));
        } catch (const std::out_of_range &e) {
            std::cerr << "[ERROR]: Document with ID " << doc_id << " not found!" << std::endl;
        }
    }

    return result;
}

/**
 * Update documents in the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param preprocessor Preprocessor object instance
 * @param doc_ids Document IDs
 * @param docs Documents to update
 */
void update_docs(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, Preprocessor &preprocessor, std::vector<int> &doc_ids, std::vector<Document> &docs) {
    auto &indexer = indexer_cache_pair.first;
    auto &doc_cache = indexer_cache_pair.second;

    auto tokenized_docs = preprocess_documents(preprocessor, docs);

    indexer.update_docs(doc_ids, tokenized_docs);

    /* Update the cache */
    for (auto &doc : docs) {
        doc_cache.erase(doc.id);
        doc_cache.insert({doc.id, doc});
    }
}

/**
 * Remove documents from the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param doc_ids Document IDs
 */
void remove_docs(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, std::vector<int> &doc_ids) {
    auto &indexer = indexer_cache_pair.first;
    auto &doc_cache = indexer_cache_pair.second;

    indexer.remove_docs(doc_ids);

    /* Update the cache */
    for (auto &doc_id : doc_ids)
        doc_cache.erase(doc_id);
}

/**
 * Main function
 * @return Exit code
 */
int main() {
    /* Load documents */
    auto [docs, doc_cache] = load_documents("../data");

    /* Preprocess documents */
    auto preprocessor = Preprocessor();
    auto tokenized_docs = preprocess_documents(preprocessor, docs);

    /* Index documents */
    auto indexer = Indexer(tokenized_docs);
    std::pair<Indexer, std::unordered_map<int, Document>> indexer_cache_pair = {indexer, doc_cache};

    std::vector<int> doc_ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 100000};
    auto result = get_docs(indexer_cache_pair, doc_ids);

//    /* Search "Geralt z Rivie" */
//    std::string query = "Geralt z Rivie";
//    auto query_tokens = preprocessor.preprocess_text(query, true, false);
//    auto result = indexer.search(query_tokens, 3);
//    print_query_results(query, result, indexer, doc_cache);
//
//    /* Update the document, so it contains "Geralt z Rivie" more and so it is way more relevant (see Score print) */
//    std::cout << "Updating document with ID 550..." << std::endl;
//    indexer.update_doc(550, TokenizedDocument(550, {"uz", "ne", "geralt", "z", "rivie"}, {}, {}, {}, {}, {"geralt", "z", "rivie", "geralt", "z", "rivie", "geralt", "z", "rivie", "geralt", "z", "rivie"}));
//
//    /* Search "Geralt z Rivie" again */
//    result = indexer.search(query_tokens, 3);
//    print_query_results(query, result, indexer, doc_cache);
//
//    /* Remove the document completely now and see how the score of others changes too, because overall IDF changes */
//    std::cout << "Removing document with ID 550..." << std::endl;
//    indexer.remove_doc(550);
//
//    /* Search "Geralt z Rivie" again and see how the score of others changes (also ID 550 is gone, shocking!) */
//    result = indexer.search(query_tokens, 10);
//    print_query_results(query, result, indexer, doc_cache);
//
//    /* Search "Geralt z Rivie" boolean */
//    query = "NOT Geralt OR (z AND NOT NOT Rivie)";
//    auto bool_tokens = preprocessor.parse_bool_query(query);
//
//    std::cout << query << std::endl;
//
//    for (auto &token : bool_tokens)
//        std::cout << token << " ";
//    std::cout << std::endl;
//
//    auto result_ids = indexer.search(bool_tokens);
//    std::cout << "Documents that contain \"" << query << "\":" << std::endl;
//    std::cout << "Found " << result_ids.size() << " documents" << std::endl << "Results:" << std::endl;
//    for (auto &doc_id : result_ids)
//        std::cout << "ID: " << doc_id << ", Title: " << doc_cache.at(doc_id).title << std::endl;
//    std::cout << std::endl;

    return EXIT_SUCCESS;
}
