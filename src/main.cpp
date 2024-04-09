#include <iostream>
#include <chrono>
#include "DataLoader.h"
#include "Preprocessor.h"
#include "Indexer.h"

/**
 * Load documents from the given directory
 * @param dir_path Directory path
 * @param verbose Whether to print the progress
 * @return Documents as a vector
 */
std::vector<Document> load_documents(const std::string &dir_path, bool verbose=true) {
    if (verbose)
        std::cout << "Loading documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();
    
    auto docs = DataLoader::load_json_documents_from_dir("../data");

    auto t_end = std::chrono::high_resolution_clock::now();
    if (verbose) {
        std::cout << "Loaded " << docs.size() << " documents" << std::endl;
        std::cout << "Loading done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
    }

    return docs;
}

/**
 * Preprocess documents (Documents -> TokenizedDocuments)
 * @param preprocessor Preprocessor object instance
 * @param docs Documents to preprocess
 * @param verbose Whether to print the progress
 * @return Tokenized documents (preprocessed)
 */
std::vector<TokenizedDocument> preprocess_documents(Preprocessor &preprocessor, std::vector<Document> &docs, bool verbose=true) {
    if (verbose)
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
    if (verbose) {
        std::cout << "Preprocessed " << tokenized_docs.size() << " documents" << std::endl;
        std::cout << "Preprocessing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
    }

    return tokenized_docs;
}

/**
 * Add documents to the indexer and cache
 * @param indexer Indexer
 * @param preprocessor Preprocessor object instance
 * @param docs Documents to add
 * @param verbose Whether to print the progress
 */
void add_docs(Indexer &indexer, Preprocessor &preprocessor, std::vector<Document> &docs, bool verbose=true) {
    if (verbose) {
        std::cout << "Adding new documents..." << std::endl << "IDs: ";
        for (auto &doc : docs)
            std::cout << doc.id << ", ";
        std::cout << std::endl;
    }

    auto tokenized_docs = preprocess_documents(preprocessor, docs, false);

    indexer.add_docs(docs, tokenized_docs);
}

/**
 * Get documents from the indexer and cache
 * @param indexer Indexer
 * @param doc_ids Document IDs
 * @param verbose Whether to print the progress
 * @return Documents
 */
std::vector<Document> get_docs(Indexer &indexer, std::vector<int> &doc_ids, bool verbose=true) {
    if (verbose) {
        std::cout << "Getting documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    auto result = indexer.get_docs(doc_ids);

    return result;
}

/**
 * Update documents in the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param preprocessor Preprocessor object instance
 * @param doc_ids Document IDs
 * @param docs Documents to update
 * @param verbose Whether to print the progress
 */
void update_docs(Indexer &indexer, Preprocessor &preprocessor, std::vector<int> &doc_ids, std::vector<Document> &docs, bool verbose=true) {
    if (verbose) {
        std::cout << "Updating documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    auto tokenized_docs = preprocess_documents(preprocessor, docs, false);

    indexer.update_docs(doc_ids, docs, tokenized_docs);
}

/**
 * Remove documents from the indexer and cache
 * @param indexer_cache_pair Indexer and cache pair
 * @param doc_ids Document IDs
 * @param verbose Whether to print the progress
 */
void remove_docs(Indexer &indexer, std::vector<int> &doc_ids, bool verbose=true) {
    if (verbose) {
        std::cout << "Removing documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

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
std::pair<std::vector<Document>, std::vector<float>> search(Indexer &indexer, Preprocessor &preprocessor, std::string &query, int k, bool print=true) {
    auto query_tokens = preprocessor.preprocess_text(query, true, false);

    auto result = indexer.search(query_tokens, k);

    auto result_docs = get_docs(indexer, result.first, false);

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
std::vector<Document> search(Indexer &indexer, Preprocessor &preprocessor, std::string &query, bool print=true) {
    std::cout << "Query: " << query << std::endl << "Postfix notation: ";
    auto bool_tokens = preprocessor.parse_bool_query(query);
    for (auto &token : bool_tokens)
        std::cout << token << " ";
    std::cout << std::endl;

    auto result_ids = indexer.search(bool_tokens);

    auto result_docs = get_docs(indexer, result_ids, false);

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
    auto docs = load_documents("../data");

    /* Preprocess documents */
    auto preprocessor = Preprocessor();
    auto tokenized_docs = preprocess_documents(preprocessor, docs);

    /* Index documents */
    auto indexer = Indexer(docs, tokenized_docs);

    /* Search "Geralt z Rivie" */
    std::string query = "Geralt z Rivie";
    auto result = search(indexer, preprocessor, query, 3);

    /* Update the document, so it contains "Geralt z Rivie" more and so it is way more relevant (see Score print) */
    std::vector<int> new_doc_ids = {550};
    std::vector<Document> new_docs = {{550, {"už ne Geralt z Rivie"}, {}, {}, {}, {}, {"Geralt z Rivie Geralt z Rivie Geralt z Rivie Geralt z Rivie"}}};
    update_docs(indexer, preprocessor, new_doc_ids, new_docs);

    /* Search "Geralt z Rivie" again */
    result = search(indexer, preprocessor, query, 3);

    /* Remove the document completely now and see how the score of others changes too, because overall IDF changes */
    remove_docs(indexer, new_doc_ids);

    /* Search "Geralt z Rivie" again and see how the score of others changes (also ID 550 is gone, shocking!) */
    result = search(indexer, preprocessor, query, 3);

    /* Search "Geralt z Rivie" boolean */
    query = "NOT Geralt AND (z OR NOT NOT Rivie)";
    auto result_bool = search(indexer, preprocessor, query);

    return EXIT_SUCCESS;
}
