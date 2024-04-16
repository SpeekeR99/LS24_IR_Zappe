#pragma once

#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include "nlohmann/json.hpp"
#include "TF_IDF.h"
#include "Preprocessor.h"
#include "PyHandler.h"

using json = nlohmann::json;

/** Language detection is really slow */
constexpr bool DETECT_LANG = false;

/**
 * Enum for the field type
 */
enum class FieldType {
    TITLE,
    CONTENT,
    ALL
};

/**
 * Class for indexing the documents
 */
class Indexer {
private:
    /** Collection of documents */
    std::vector<TokenizedDocument> collection;
    /** Keywords */
    std::unordered_set<std::string> keywords;
    /** Main index */
    std::map<std::string, map_element> index;
    /** Title index */
    std::map<std::string, map_element> title_index;
    /** Document norms (cosine similarity) */
    std::map<int, float> norms;
    /** Title norms (cosine similarity) */
    std::map<int, float> title_norms;
    /** Map of word -> (doc_id, positions) */
    std::map<std::string, std::map<int, std::vector<int>>> positions_map;

    /**
     * Adds words from the collection to the keywords
     */
    void docs_to_keywords();

    /**
     * Index the given collection of documents
     */
    void index_everything();

public:
    /** Document cache */
    std::unordered_map<int, Document> doc_cache;

    /**
     * Constructor for the Indexer class
     */
    Indexer();
    /**
     * Constructor for the Indexer class
     * @param original_collection Original collection of documents
     * @param tokenized_collection Tokenized collection of documents
     * @param positions_map Map of word -> (doc_id, positions)
     */
    Indexer(const std::vector<Document> &original_collection, const std::vector<TokenizedDocument> &tokenized_collection, std::map<std::string, std::map<int, std::vector<int>>> &positions_map);

    /**
     * Add documents to the collection
     * @param docs Documents to add
     */
    void add_docs(const std::vector<Document> &docs, const std::vector<TokenizedDocument> &tokenized_docs, std::map<std::string, std::map<int, std::vector<int>>> &positions_map);
    /**
     * Get the document with the given ID
     * @param doc_id Document ID
     * @return Document with the given ID
     */
    Document get_doc(int doc_id);
    /**
     * Get the tokenized document with the given ID
     * @param doc_id Document ID
     * @return Tokenized document with the given ID
     */
    TokenizedDocument get_tokenized_doc(int doc_id);
    /**
     * Get documents with the given IDs
     * @param doc_ids Vector of document IDs
     * @return Vector of documents with the given IDs
     */
    std::vector<Document> get_docs(const std::vector<int> &doc_ids);
    /**
     * Update documents with the given IDs
     * @param doc_ids Vector of document IDs
     * @param docs Vector of new documents
     */
    void update_docs(const std::vector<int> &doc_ids, const std::vector<Document> &docs, const std::vector<TokenizedDocument> &tokenized_docs, std::map<std::string, std::map<int, std::vector<int>>> &positions_map);
    /**
     * Remove documents with the given IDs
     * @param doc_ids Vector of document IDs
     */
    void remove_docs(const std::vector<int> &doc_ids);

    /**
     * Calculate the cosine similarity between the given query and the document with the given ID
     * @param query Query
     * @param doc_id Document ID
     * @param title Whether to use the title index
     * @return Cosine similarity
     */
    [[nodiscard]] float cosine_similarity(const std::map<std::string, float> &query, int doc_id, bool title = false) const;
    /**
     * Search for the given query (VECTOR MODEL)
     * @param query Query tokens
     * @param k Top k results
     * @param field Field to search in
     * @return IDs of the top k documents and their scores and positions
     */
    [[nodiscard]] std::tuple<std::vector<int>, std::vector<float>, std::map<std::string, std::map<int, std::vector<int>>>> search(const std::vector<std::string> &query, int k, FieldType field = FieldType::ALL) const;
    /**
     * Search for the given query (BOOLEAN MODEL)
     * @param query_tokens Query tokens (EXPECTED postfix notation)
     * @param field Field to search in
     * @return IDs of the documents that fulfill the query conditions and their positions
     */
    [[nodiscard]] std::tuple<std::vector<int>, std::map<std::string, std::map<int, std::vector<int>>>> search(const std::vector<std::string> &query_tokens, FieldType field = FieldType::ALL) const;

    /**
     * Indexer to json
     * @return JSON representation of the indexer
     */
    [[nodiscard]] json to_json() const;
    /**
     * Load indexer from json
     * @param j JSON representation of the indexer
     */
    void from_json(const json &j);

    /**
     * Get the size of the collection
     * @return Size of the collection
     */
    [[nodiscard]] int get_collection_size() const;
    /**
     * Get the size of the index
     * @return Size of the index
     */
    [[nodiscard]] int get_index_size() const;
    /**
     * Get the size of the title index
     * @return Size of the title index
     */
    [[nodiscard]] int get_title_index_size() const;
};
