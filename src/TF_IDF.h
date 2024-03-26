#pragma once

#include <map>
#include <algorithm>
#include <cmath>
#include "Document.h"

/**
 * Map element used in calc_tf_idf
 */
struct map_element {
    /** IDF value of a word */
    float idf;
    /** Document ID and TF-IDF value for a word */
    std::vector<std::pair<int, float>> doc_tf_idf;
};

/**
 * TF-IDF class
 */
class TF_IDF {
public:
    /**
     * Calculate IDF from DF
     * @param collection Collection of documents
     * @param title Whether to calculate IDF for titles
     * @return Map of words and their IDF values
     */
    static std::map<std::string, float> calc_idf(const std::vector<TokenizedDocument> &collection, bool title = false);
    /**
     * Calculate TF from a document
     * @param doc Document
     * @return Map of words and their TF values
     */
    static std::map<std::string, float> calc_tf(const std::vector<std::string> &doc);
    /**
     * Calculate TF-IDF from a collection of documents
     * @param collection Collection of documents
     * @param norms Norms of documents
     * @param title Whether to calculate TF-IDF for titles
     * @return Map of words and their IDF values
     */
    static std::map<std::string, map_element> calc_tf_idf(const std::vector<TokenizedDocument> &collection, std::map<int, float> &norms, bool title = false);
};
