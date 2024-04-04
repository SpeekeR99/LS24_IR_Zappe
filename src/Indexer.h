#pragma once

#include "TF_IDF.h"

/**
 * Class for indexing the documents
 */
class Indexer {
private:
    /** Collection of documents */
    std::vector<TokenizedDocument> collection;
    /** Main index */
    std::map<std::string, map_element> index;
    /** Title index */
    std::map<std::string, map_element> title_index;
    /** Document norms (cosine similarity) */
    std::map<int, float> norms;
    /** Title norms (cosine similarity) */
    std::map<int, float> title_norms;

    /**
     * Index the given collection of documents
     */
    void index_everything();

public:
    /**
     * Constructor for the Indexer class
     */
    Indexer();
    /**
     * Constructor for the Indexer class
     * @param collection Collection of documents
     */
    explicit Indexer(const std::vector<TokenizedDocument> &collection);

    /**
     * Add a document to the collection
     * @param doc Document to add
     */
    void add_doc(const TokenizedDocument &doc);
    /**
     * Add documents to the collection
     * @param docs Documents to add
     */
    void add_docs(const std::vector<TokenizedDocument> &docs);
    /**
     * Get the document with the given ID
     * @param doc_id Document ID
     * @return Document with the given ID
     */
    TokenizedDocument get_doc(int doc_id);
    /**
     * Get documents with the given IDs
     * @param doc_ids Vector of document IDs
     * @return Vector of documents with the given IDs
     */
    std::vector<TokenizedDocument> get_docs(const std::vector<int> &doc_ids);
    /**
     * Update the document with the given ID
     * @param doc_id Document ID
     * @param doc New document
     */
    void update_doc(int doc_id, const TokenizedDocument &doc);
    /**
     * Update documents with the given IDs
     * @param doc_ids Vector of document IDs
     * @param docs Vector of new documents
     */
    void update_docs(const std::vector<int> &doc_ids, const std::vector<TokenizedDocument> &docs);
    /**
     * Remove the document with the given ID
     * @param doc_id Document ID
     */
    void remove_doc(int doc_id);
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
     * @param query Query
     * @param k Top k results
     * @return IDs of the top k documents and their scores
     */
    [[nodiscard]] std::pair<std::vector<int>, std::vector<float>> search(const std::vector<std::string> &query, int k) const;
    /**
     * Search for the given query (BOOLEAN MODEL)
     * @param query Query
     * @return IDs of the documents that fulfill the query conditions
     */
    [[nodiscard]] std::vector<int> search(const std::vector<std::string> &query);

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
