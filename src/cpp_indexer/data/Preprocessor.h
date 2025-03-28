#pragma once

#include <string>
#include <regex>
#include <vector>
#include <utility>
#include <numeric>
#include <set>
#include <map>
#include <tuple>

#include "Const.h"
#include "libstemmer.h"
#include "RdrLemmatizer.h"
#include "sl_lemmatizer.h"

/**
 * Operator for boolean queries
 */
enum Operator {
    AND = 0,
    OR,
    NOT
};

/**
 * Map of operators and their string representations
 */
static const char * const operators_map[] = {
    [AND] = "AND",
    [OR] = "OR",
    [NOT] = "NOT"
};

/**
 * Class for preprocessing the Witcher crawled data
 */
class Preprocessor {
private:
    /** Language settings for the stemmer */
    const std::string language = "czech";
    /** Filepath to czech binary file for the lemmatizer */
    const std::string lemma_czech_bin = "../lib/lemma/lemmagen/dictionaries/czech.bin";
    /** Lemmatizer instance for the czech language */
    RdrLemmatizer *lemmatizer = nullptr;
    /** Stemmer instance for the czech language */
    struct sb_stemmer *stemmer = nullptr;
    /** Filepath to the file with stopwords */
    const std::string stopwords_file = "../src/czech.stop";
    /** Stopwords */
    std::set<std::string> stopwords;

    /**
     * Prepare the stemmer for the given language
     * @return True if stemmer was created, false otherwise
    */
    bool prepare_stemmer();
    /**
     * Prepare the lemmatizer for the given language
     * @return True if lemmatizer was created, false otherwise
    */
    bool prepare_lemmatizer();

    /**
     * Load stopwords from the given file
     * @param stopwords_file Filepath to the file with stopwords
     * @return True if stopwords were loaded, false otherwise
     */
    bool load_stopwords();

    /**
     * Get the precedence of the given boolean operator
     * @param op Boolean operator
     * @return Precedence of the operator
     */
    int bool_op_precedence(const std::string &op);

public:
    /**
     * Constructor for the Preprocessor class
    */
    Preprocessor();
    /**
     * Destructor for the Preprocessor class
    */
    ~Preprocessor();

    /**
     * Convert the given text to lower case
     * @param text Input text
     * @return Lower case text
    */
    std::string &to_lower(std::string &text);
    /**
     * Remove HTML tags from the given text
     * @param text Input text
     * @return Text without HTML tags
     */
    std::string &remove_html_tags(std::string &text);
    /**
     * Remove czech diacritics from the given text
     * @param text Input text
     * @return Text without czech diacritics
     */
    std::string &remove_special_characters(std::string &text);
    /**
     * Tokenize the given text and return the tokens with their positions
     * @param text Input text
     * @return Tokens
     */
    std::pair<std::vector<std::string>, std::map<std::string, std::vector<int>>> tokenize(const std::string &text);
    /**
     * Stem the given word
     * @param word Input word
     * @return Stemmed word
     */
    std::string stem(const std::string &word);
    /**
     * Stem the given vector of words
     * @param words Input vector of words
     * @return Stemmed vector of words
     */
    std::vector<std::string> stem(const std::vector<std::string> &words);
    /**
     * Lemmatize the given word
     * @param word Input word
     * @return Lemmatized word
     */
    std::string lemmatize(const std::string &word);
    /**
     * Lemmatize the given vector of words
     * @param words Input vector of words
     * @return Lemmatized vector of words
     */
    std::vector<std::string> lemmatize(const std::vector<std::string> &words);
    /**
     * Remove stopwords from the given vector of words
     * @param words Input vector of words
     * @return Vector of words without stopwords
     */
    std::vector<std::string> &remove_stopwords(std::vector<std::string> &words);
    /**
     * Remove duplicates from the given vector of words
     * @param words Input vector of words
     * @return Vector of words without duplicates
     */
    std::vector<std::string> &remove_duplicates(std::vector<std::string> &words);
    /**
     * Preprocess the given text
     * @param text Input text
     * @param content Whether to preprocess the content of the text
     * @return Preprocessed text with tokens and their positions
     */
    std::pair<std::vector<std::string>, std::map<std::string, std::vector<int>>> preprocess_text(std::string text, bool content=false);
    /**
     * Preprocess the given vector of words, phrases or sentences
     * @param text Input vector of words, phrases or sentences
     * @param content Whether to preprocess the content of the text
     * @return Preprocessed text with tokens and their positions
     */
    std::pair<std::vector<std::string>, std::map<std::string, std::vector<int>>> preprocess_text(const std::vector<std::string> &text, bool content=false);

    /**
     * Parse the given boolean query into postfix notation
     * @param query Boolean query
     * @return Postfix notation of the query
     */
    std::vector<std::string> parse_bool_query(const std::string &query);
};

