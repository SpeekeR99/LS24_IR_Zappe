#include <iostream>
#include <chrono>
#include "DataLoader.h"
#include "Preprocessor.h"
#include "Indexer.h"

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
    std::cout << "Adding new documents..." << std::endl << "IDs: ";
    for (auto &doc : docs)
        std::cout << doc.id << ", ";
    std::cout << std::endl;

    auto &indexer = indexer_cache_pair.first;
    auto &doc_cache = indexer_cache_pair.second;

    /* Update the cache */
    for (auto &doc : docs)
        doc_cache.insert({doc.id, doc});

    auto tokenized_docs = preprocess_documents(preprocessor, docs);

    indexer.add_docs(tokenized_docs);
}

/**
 * Get documents from the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param doc_ids Document IDs
 * @return Documents
 */
std::vector<Document> get_docs(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, std::vector<int> &doc_ids) {
    std::cout << "Getting documents..."  << std::endl << "IDs: ";
    for (auto &doc_id : doc_ids)
        std::cout << doc_id << ", ";
    std::cout << std::endl;

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
    std::cout << "Updating documents..."  << std::endl << "IDs: ";
    for (auto &doc_id : doc_ids)
        std::cout << doc_id << ", ";
    std::cout << std::endl;

    auto &indexer = indexer_cache_pair.first;
    auto &doc_cache = indexer_cache_pair.second;

    /* Update the cache */
    for (auto &doc : docs) {
        doc_cache.erase(doc.id);
        doc_cache.insert({doc.id, doc});
    }

    auto tokenized_docs = preprocess_documents(preprocessor, docs);

    indexer.update_docs(doc_ids, tokenized_docs);
}

/**
 * Remove documents from the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param doc_ids Document IDs
 */
void remove_docs(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, std::vector<int> &doc_ids) {
    std::cout << "Removing documents..."  << std::endl << "IDs: ";
    for (auto &doc_id : doc_ids)
        std::cout << doc_id << ", ";
    std::cout << std::endl;

    auto &indexer = indexer_cache_pair.first;
    auto &doc_cache = indexer_cache_pair.second;

    /* Update the cache */
    for (auto &doc_id : doc_ids)
        doc_cache.erase(doc_id);

    indexer.remove_docs(doc_ids);
}

/**
 * Print the search results for the given query (Vector space model)
 * @param query Query
 * @param result Result
 * @param indexer Indexer
 * @param doc_cache Document cache
 */
void print_query_results(const std::string &query, const std::pair<std::vector<Document>, std::vector<float>> &result) {
    std::cout << "Top " << result.first.size() << " search results for \"" << query << "\":" << std::endl;
    for (auto i = 0; i < result.first.size(); i++)
        std::cout << "Rank: " << i + 1 << ", ID: " << result.first[i].id << ", Title: " << result.first[i].title << ", Score: " << result.second[i] << std::endl;
    std::cout << std::endl;
}

/**
 * Print the search results for the given query (Boolean model)
 * @param query  Query
 * @param result Result
 */
void print_query_results(const std::string &query, std::vector<Document> &result) {
    std::cout << "Documents that contain \"" << query << "\":" << std::endl;
    std::cout << "Found " << result.size() << " documents" << std::endl << "Results:" << std::endl;
    for (auto &doc : result)
        std::cout << "ID: " << doc.id << ", Title: " << doc.title << std::endl;
    std::cout << std::endl;
}

/**
 * Search for the given query (Vector space model)
 * @param indexer_cache_pair Indexer and cache pair
 * @param preprocessor Preprocessor object instance
 * @param query Query
 * @param k Number of results
 * @param print Whether to print the results
 * @return Pair of documents and scores
 */
std::pair<std::vector<Document>, std::vector<float>> search(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, Preprocessor &preprocessor, std::string &query, int k, bool print=true) {
    auto &indexer = indexer_cache_pair.first;
    auto &doc_cache = indexer_cache_pair.second;

    auto query_tokens = preprocessor.preprocess_text(query, true, false);

    auto result = indexer.search(query_tokens, k);

    auto result_docs = get_docs(indexer_cache_pair, result.first);

    if (print)
        print_query_results(query, {result_docs, result.second});

    return {result_docs, result.second};
}

/**
 * Search for the given query (Boolean model)
 * @param indexer_cache_pair Indexer and cache pair
 * @param preprocessor Preprocessor object instance
 * @param query Query
 * @param print Whether to print the results
 * @return Documents
 */
std::vector<Document> search(std::pair<Indexer, unordered_map<int, Document>> &indexer_cache_pair, Preprocessor &preprocessor, std::string &query, bool print=true) {
    auto &indexer = indexer_cache_pair.first;

    auto bool_tokens = preprocessor.parse_bool_query(query);
    for (auto &token : bool_tokens)
        std::cout << token << " ";
    std::cout << std::endl;

    auto result_ids = indexer.search(bool_tokens);

    auto result_docs = get_docs(indexer_cache_pair, result_ids);

    if (print)
        print_query_results(query, result_docs);

    return result_docs;
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

    /* Search "Geralt z Rivie" */
    std::string query = "Geralt z Rivie";
    auto result = search(indexer_cache_pair, preprocessor, query, 3);

    /* Update the document, so it contains "Geralt z Rivie" more and so it is way more relevant (see Score print) */
    std::cout << "Updating document with ID 550..." << std::endl;
    std::vector<int> new_doc_ids = {550};
    std::vector<Document> new_docs = {{550, {"už ne Geralt z Rivie"}, {}, {}, {}, {}, {"Geralt z Rivie Geralt z Rivie Geralt z Rivie Geralt z Rivie"}}};
    update_docs(indexer_cache_pair, preprocessor, new_doc_ids, new_docs);

    /* Search "Geralt z Rivie" again */
    result = search(indexer_cache_pair, preprocessor, query, 3);

    /* Remove the document completely now and see how the score of others changes too, because overall IDF changes */
    remove_docs(indexer_cache_pair, new_doc_ids);

    /* Search "Geralt z Rivie" again and see how the score of others changes (also ID 550 is gone, shocking!) */
    result = search(indexer_cache_pair, preprocessor, query, 3);

    /* Search "Geralt z Rivie" boolean */
    query = "NOT Geralt OR (z AND NOT NOT Rivie)";
    auto result_bool = search(indexer_cache_pair, preprocessor, query);

    return EXIT_SUCCESS;
}
