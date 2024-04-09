#include <iostream>
#include <chrono>
#include "DataLoader.h"
#include "Preprocessor.h"
#include "Indexer.h"

class IndexHandler {
public:
    static Preprocessor preprocessor;

    /**
     * Load documents from the given directory
     * @param dir_path Directory path
     * @param verbose Whether to print the progress
     * @return Documents as a vector
     */
    static std::vector<Document> load_documents(const std::string &dir_path, bool verbose=true);

    /**
     * Preprocess documents (Documents -> TokenizedDocuments)
     * @param preprocessor Preprocessor object instance
     * @param docs Documents to preprocess
     * @param verbose Whether to print the progress
     * @return Tokenized documents (preprocessed)
     */
    static std::vector<TokenizedDocument> preprocess_documents(std::vector<Document> &docs, bool verbose=true);

    /**
     * Save the index to the given path
     * @param indexer Indexer
     * @param index_path Index path
     */
    static void save_index(Indexer &indexer, const std::string &index_path);

    /**
     * Load the index from the given path
     * @param indexer Indexer
     * @param index_path Index path
     */
    static void load_index(Indexer &indexer, const std::string &index_path);

    /**
     * Add a document to the indexer and cache it from the given URL
     * @param indexer Indexer
     * @param url URL to download the document from
     * @param verbose Whether to print the progress
     */
    static void add_doc_url(Indexer &indexer, const std::string &url, bool verbose=true);

    /**
     * Add documents to the indexer and cache
     * @param indexer Indexer
     * @param preprocessor Preprocessor object instance
     * @param docs Documents to add
     * @param verbose Whether to print the progress
     */
    static void add_docs(Indexer &indexer, std::vector<Document> &docs, bool verbose=true);

    /**
     * Get documents from the indexer and cache
     * @param indexer Indexer
     * @param doc_ids Document IDs
     * @param verbose Whether to print the progress
     * @return Documents
     */
    static std::vector<Document> get_docs(Indexer &indexer, std::vector<int> &doc_ids, bool verbose=true);

    /**
     * Update documents in the indexer and cache
     * @param indexer_cache_pair Indexer and cache pair
     * @param preprocessor Preprocessor object instance
     * @param doc_ids Document IDs
     * @param docs Documents to update
     * @param verbose Whether to print the progress
     */
    static void update_docs(Indexer &indexer, std::vector<int> &doc_ids, std::vector<Document> &docs, bool verbose=true);

    /**
     * Remove documents from the indexer and cache
     * @param indexer_cache_pair Indexer and cache pair
     * @param doc_ids Document IDs
     * @param verbose Whether to print the progress
     */
    static void remove_docs(Indexer &indexer, std::vector<int> &doc_ids, bool verbose=true);

    /**
     * Print the search results for the given query (Vector space model)
     * @param query Query
     * @param result Result
     * @param indexer Indexer
     * @param doc_cache Document cache
     */
    static void print_query_results(const std::string &query, const std::pair<std::vector<Document>, std::vector<float>> &result);

    /**
     * Print the search results for the given query (Boolean model)
     * @param query  Query
     * @param result Result
     */
    static void print_query_results(const std::string &query, std::vector<Document> &result);

    /**
     * Search for the given query (Vector space model)
     * @param indexer_cache_pair Indexer and cache pair
     * @param preprocessor Preprocessor object instance
     * @param query Query
     * @param k Number of results
     * @param print Whether to print the results
     * @return Pair of documents and scores
     */
    static std::pair<std::vector<Document>, std::vector<float>> search(Indexer &indexer, std::string &query, int k, bool print=true);

    /**
     * Search for the given query (Boolean model)
     * @param indexer_cache_pair Indexer and cache pair
     * @param preprocessor Preprocessor object instance
     * @param query Query
     * @param print Whether to print the results
     * @return Documents
     */
    static std::vector<Document> search(Indexer &indexer, std::string &query, bool print=true);
};
