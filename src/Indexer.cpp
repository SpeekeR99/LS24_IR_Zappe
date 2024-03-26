#include "Indexer.h"

Indexer::Indexer() : collection(std::vector<TokenizedDocument>()), index(std::map<std::string, map_element>()), norms(std::map<int, float>()) {
    /* Nothing to do here :) */
}

Indexer::Indexer(const std::vector<TokenizedDocument> &collection) : collection(std::vector<TokenizedDocument>()), index(std::map<std::string, map_element>()), norms(std::map<int, float>()) {
    this->add_docs(collection);
}

void Indexer::index_everything() {
    this->norms.clear();
    this->title_norms.clear();
    this->index = TF_IDF::calc_tf_idf(this->collection, this->norms);
    this->title_index = TF_IDF::calc_tf_idf(this->collection, this->title_norms, true);
}


void Indexer::add_doc(const TokenizedDocument &doc) {
    this->collection.emplace_back(doc);
    this->index_everything();
}

void Indexer::add_docs(const std::vector<TokenizedDocument> &docs) {
    for (const auto &doc : docs)
        this->collection.emplace_back(doc);
    this->index_everything();
}

TokenizedDocument Indexer::get_doc(int doc_id) {
    for (const auto &doc : this->collection)
        if (doc.id == doc_id)
            return doc;
    return TokenizedDocument(-1, {"NOT FOUND"}, {}, {}, {}, {}, {});
}

std::vector<TokenizedDocument> Indexer::get_docs(const std::vector<int> &doc_ids) {
    std::vector<TokenizedDocument> result;
    result.reserve(doc_ids.size());
    for (const auto &doc_id : doc_ids)
        for (const auto &doc : this->collection)
            if (doc.id == doc_id)
                result.emplace_back(doc);
    return result;
}

void Indexer::update_doc(int doc_id, const TokenizedDocument &doc) {
    for (auto &d : this->collection)
        if (d.id == doc_id)
            d = doc;
    this->index_everything();
}

void Indexer::update_docs(const std::vector<int> &doc_ids, const std::vector<TokenizedDocument> &docs) {
    for (int i = 0; i < doc_ids.size(); i++)
        for (auto &d : this->collection)
            if (d.id == doc_ids[i]) {
                d = docs[i];
                break;
            }
    this->index_everything();
}

void Indexer::remove_doc(int doc_id) {
    for (auto it = this->collection.begin(); it != this->collection.end(); it++)
        if (it->id == doc_id) {
            this->collection.erase(it);
            break;
        }
    this->index_everything();
}

void Indexer::remove_docs(const std::vector<int> &doc_ids) {
    for (const auto &doc_id : doc_ids)
        for (auto it = this->collection.begin(); it != this->collection.end(); it++)
            if (it->id == doc_id) {
                this->collection.erase(it);
                break;
            }
    this->index_everything();
}

float Indexer::cosine_similarity(const std::map<std::string, float> &query, int doc_id, bool title) const {
    /* Initialize dot product and norms */
    float dot = 0;
    float norm_query = 0;
    float norm_doc = this->norms.at(doc_id);
    if (title)
        norm_doc = this->title_norms.at(doc_id);

    /* Iterate over TF-IDF of words in query */
    for (const auto& [word, value] : query) {
        /* If word is found in a document use it for dot product */
        if (title && this->title_index.find(word) != this->title_index.end()) {
            for (const auto &[id, tf_idf]: this->title_index.at(word).doc_tf_idf)
                if (id == doc_id)
                    dot += value * tf_idf;
        } else if (!title && this->index.find(word) != this->index.end()) {
                for (const auto &[id, tf_idf]: this->index.at(word).doc_tf_idf)
                    if (id == doc_id)
                        dot += value * tf_idf;
        }
        norm_query += value * value;
    }

    /* Calculate cosine similarity */
    return dot / (std::sqrt(norm_query) * norm_doc);
}

std::pair<std::vector<int>, std::vector<float>> Indexer::search(const std::vector<std::string> &query, int k) const {
    /* Calculate TF for the query */
    auto tf_query = TF_IDF::calc_tf(query);
    std::map<std::string, float> tf_idf_query;

    /* Calculate TF-IDF for the query */
    for (const auto& [word, value] : tf_query)
        if (this->index.find(word) != this->index.end())
            tf_idf_query[word] = value * this->index.at(word).idf;
        else
            tf_idf_query[word] = 0;

    /* Calculate cosine similarity for each document */
    std::vector<std::pair<int, float>> title_results;
    for (const auto& doc: this->collection)
        title_results.emplace_back(doc.id, this->cosine_similarity(tf_idf_query, doc.id, true));
    std::vector<std::pair<int, float>> results;
    for (const auto& doc: this->collection)
        results.emplace_back(doc.id, this->cosine_similarity(tf_idf_query, doc.id));

    /* Combine results from title and content, title matches are weighted more */
    const float title_weight = 1.5;
    for (const auto& [doc_id, value] : results)
        results[doc_id].second += title_weight * title_results[doc_id].second;

    /* Sort results by cosine similarity */
    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    /* Return top k results */
    std::pair<std::vector<int>, std::vector<float>> top_k;
    for (int i = 0; i < k; i++) {
        top_k.first.emplace_back(results[i].first);
        top_k.second.emplace_back(results[i].second);
    }

    return top_k;
}

int Indexer::get_collection_size() const {
    return static_cast<int>(this->collection.size());
}

int Indexer::get_index_size() const {
    return static_cast<int>(this->index.size());
}

int Indexer::get_title_index_size() const {
    return static_cast<int>(this->title_index.size());
}
