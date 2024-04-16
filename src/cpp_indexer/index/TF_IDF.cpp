#include "TF_IDF.h"

std::map<std::string, float> TF_IDF::calc_idf(const std::vector<TokenizedDocument> &collection, bool title) {
    /* Initialize IDF */
    std::map<std::string, float> idf;

    /* Iterate over documents */
    for (const auto &doc : collection) {
        /* Split words */
        auto words = doc.content;
        words.insert(words.end(), doc.toc.begin(), doc.toc.end());
        words.insert(words.end(), doc.h1.begin(), doc.h1.end());
        words.insert(words.end(), doc.h2.begin(), doc.h2.end());
        words.insert(words.end(), doc.h3.begin(), doc.h3.end());
        if (title)
            words = doc.title;
        /* Store found words */
        std::vector<std::string> found_in_this_doc;

        /* Iterate over words */
        for (const auto &word : words)
            /* If word is not found in this document */
            if (std::find(found_in_this_doc.begin(), found_in_this_doc.end(), word) == found_in_this_doc.end()) {
                /* Increment DF and mark word as found */
                idf[word] += 1;
                found_in_this_doc.push_back(word);
            }
    }

    /* Calculate IDF from DF */
    for (auto& [word, count] : idf)
        count = std::log10(static_cast<float>(collection.size()) / count);

    return idf;
}

std::map<std::string, float> TF_IDF::calc_tf(const std::vector<std::string> &doc) {
    /* Initialize TF */
    std::map<std::string, float> tf;

    /* Iterate over words */
    for (const auto &word : doc)
        tf[word] += 1;

    /* Calculate TF */
    for (auto& [word, count] : tf)
        count = 1 + std::log10(count);
    return tf;
}

std::map<std::string, map_element> TF_IDF::calc_tf_idf(const std::vector<TokenizedDocument> &collection, std::map<int, float> &norms, bool title) {
    auto idf = calc_idf(collection);
    std::map<int, std::map<std::string, float>> tf_idf_docs;

    for (const auto &doc : collection) {
        /* Get TF of words in document */
        auto words = doc.content;
        words.insert(words.end(), doc.toc.begin(), doc.toc.end());
        words.insert(words.end(), doc.h1.begin(), doc.h1.end());
        words.insert(words.end(), doc.h2.begin(), doc.h2.end());
        words.insert(words.end(), doc.h3.begin(), doc.h3.end());
        if (title)
            words = doc.title;
        auto tf = calc_tf(words);
        /* Calculate TF-IDF */
        std::map<std::string, float> tf_idf_temp;
        for (auto &[word, value] : tf)
            tf_idf_temp[word] = value * idf[word];
        tf_idf_docs[doc.id] = tf_idf_temp;
    }

    /* Store TF-IDF in a map */
    std::map<std::string, map_element> map_ele;
    for (const auto &[doc_id, doc_tf_idf] : tf_idf_docs) {
        float norm = 0;
        for (const auto &[word, value] : doc_tf_idf) {
            map_ele[word].doc_tf_idf.emplace_back(doc_id, value);

            norm += value * value;
        }
        norms[doc_id] = std::sqrt(norm);
    }
    /* Store IDF too, for easy query TF-IDF calculation */
    for (auto &[word, value] : map_ele)
        value.idf = idf[word];

    return map_ele;
}
