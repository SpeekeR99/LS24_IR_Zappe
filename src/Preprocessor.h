#pragma once

#include <string>
#include <regex>
#include <vector>
#include <utility>
#include <numeric>
#include <set>
#include <map>

#include "libstemmer.h"
#include "RdrLemmatizer.h"
#include "sl_lemmatizer.h"

/**
 * Class for preprocessing the Witcher crawled data
 */
class Preprocessor {
private:
    /** Language settings for the stemmer */
    const std::string language = "czech";
    /** Filepath to czech binary file for the lemmatizer */
    const std::string lemma_czech_bin = "../lib/lemma/lemmagen/dictionaries/czech.bin";
    /** Filepath to the file with stopwords */
    const std::string stopwords_file = "../src/czech.stop";

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

    /** Lemmatizer instance for the czech language */
    RdrLemmatizer *lemmatizer = nullptr;
    /** Stemmer instance for the czech language */
    struct sb_stemmer *stemmer = nullptr;

    /**
     * Load stopwords from the given file
     * @param stopwords_file Filepath to the file with stopwords
     * @return True if stopwords were loaded, false otherwise
     */
    bool load_stopwords();
    /** Stopwords */
    std::set<std::string> stopwords;

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
     * Tokenize the given text
     * @param text Input text
     * @return Tokens
     */
    std::vector<std::string> tokenize(const std::string &text);
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
     */
    std::vector<std::string> preprocess_text(std::string &text, bool lemma=true, bool content=false);
    /**
     * Preprocess the given vector of words, phrases or sentences
     */
    std::vector<std::string> preprocess_text(const std::vector<std::string> &text, bool lemma=true, bool content=false);

    /**
     * Parse the given boolean query
     * @param query Boolean query
     * @return Subqueries of the boolean query
     */
    std::vector<std::string> parse_bool_query(const std::string &query);
};

