#include "Indexer.h"

Indexer::Indexer() : collection(std::vector<TokenizedDocument>()), keywords(), doc_cache(), index(std::map<std::string, map_element>()), norms(std::map<int, float>()) {
    /* Nothing to do here :) */
}

Indexer::Indexer(const std::vector<Document> &original_collection, const std::vector<TokenizedDocument> &tokenized_collection) : collection(std::vector<TokenizedDocument>()), keywords(), doc_cache(), index(std::map<std::string, map_element>()), norms(std::map<int, float>()) {
    this->add_docs(original_collection, tokenized_collection);
}

void Indexer::docs_to_keywords() {
    /* Clear the keywords */
    this->keywords.clear();
    /* Add words from the collection to the keywords */
    for (const auto &doc : this->collection) {
        for (const auto &word : doc.title)
            this->keywords.insert(word);
        for (const auto &word : doc.toc)
            this->keywords.insert(word);
        for (const auto &word : doc.h1)
            this->keywords.insert(word);
        for (const auto &word : doc.h2)
            this->keywords.insert(word);
        for (const auto &word : doc.h3)
            this->keywords.insert(word);
        for (const auto &word : doc.content)
            this->keywords.insert(word);
    }
}

void Indexer::index_everything() {
    /* Detect languages */
    if (DETECT_LANG) {
        std::vector<Document> docs;
        for (const auto &[_, doc]: this->doc_cache)
            docs.emplace_back(doc);
        auto langs = PyHandler::detect_lang(docs);
        for (auto i = 0; i < docs.size(); i++) {
            auto id = docs[i].id;
            this->doc_cache[id].lang = langs[i];
            this->collection[i].lang = langs[i];
        }
    }

    std::cout << "Indexing documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();
    this->docs_to_keywords();
    this->norms.clear();
    this->title_norms.clear();
    this->index = TF_IDF::calc_tf_idf(this->collection, this->norms);
    this->title_index = TF_IDF::calc_tf_idf(this->collection, this->title_norms, true);

    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "Indexed " << this->get_collection_size() << " documents and " << this->get_index_size() << " words using TF-IDF" << std::endl;
    std::cout << "(Indexed " << this->get_title_index_size() << " words in titles using TF-IDF)" << std::endl;
    std::cout << "Indexing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
}


void Indexer::add_doc(const Document &doc, const TokenizedDocument &tokenized_doc) {
    this->doc_cache.insert({doc.id, doc});
    this->collection.emplace_back(tokenized_doc);
    this->index_everything();
}

void Indexer::add_docs(const std::vector<Document> &docs, const std::vector<TokenizedDocument> &tokenized_docs) {
    for (int i = 0; i < docs.size(); i++) {
        this->doc_cache.insert({docs[i].id, docs[i]});
        this->collection.emplace_back(tokenized_docs[i]);
    }
    this->index_everything();
}

Document Indexer::get_doc(int doc_id) {
    try {
        return this->doc_cache.at(doc_id);
    } catch (const std::out_of_range &e) {
        std::cerr << "[ERROR]: Document with ID " << doc_id << " not found!" << std::endl;
        return {-1, {"NOT FOUND"}, {}, {}, {}, {}, {}};
    }
}

std::vector<Document> Indexer::get_docs(const std::vector<int> &doc_ids) {
    std::vector<Document> result;
    result.reserve(doc_ids.size());
    for (const auto &doc_id : doc_ids)
        result.emplace_back(this->get_doc(doc_id));
    return result;
}

void Indexer::update_doc(int doc_id, const Document &doc, const TokenizedDocument &tokenized_doc) {
    this->doc_cache.erase(doc_id);
    this->doc_cache.insert({doc_id, doc});

    for (auto &d : this->collection)
        if (d.id == doc_id)
            d = tokenized_doc;

    this->index_everything();
}

void Indexer::update_docs(const std::vector<int> &doc_ids, const std::vector<Document> &docs, const std::vector<TokenizedDocument> &tokenized_docs) {
    for (int i = 0; i < doc_ids.size(); i++) {
        this->doc_cache.erase(doc_ids[i]);
        this->doc_cache.insert({doc_ids[i], docs[i]});

        for (auto &d : this->collection)
            if (d.id == doc_ids[i])
                d = tokenized_docs[i];
    }
    this->index_everything();
}

void Indexer::remove_doc(int doc_id) {
    try {
        this->doc_cache.at(doc_id);
    } catch (const std::out_of_range &e) {
        std::cerr << "[ERROR]: Document with ID " << doc_id << " not found!" << std::endl;
        return;
    }
    for (auto it = this->collection.begin(); it != this->collection.end(); it++)
        if (it->id == doc_id) {
            this->collection.erase(it);
            break;
        }
    this->index_everything();
}

void Indexer::remove_docs(const std::vector<int> &doc_ids) {
    for (const auto &doc_id : doc_ids) {
        try {
            this->doc_cache.at(doc_id);
        } catch (const std::out_of_range &e) {
            std::cerr << "[ERROR]: Document with ID " << doc_id << " not found!" << std::endl;
            continue;
        }
        for (auto it = this->collection.begin(); it != this->collection.end(); it++)
            if (it->id == doc_id) {
                this->collection.erase(it);
                break;
            }
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

std::pair<std::vector<int>, std::vector<float>> Indexer::search(const std::vector<std::string> &query, int k, FieldType field) const {
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

    if (field == FieldType::ALL) {
        /* Combine results from title and content, title matches are weighted more */
        const float title_weight = 1.5;
        for (const auto& [doc_id, value] : results)
            results[doc_id].second += title_weight * title_results[doc_id].second;
    } else if (field == FieldType::TITLE) {
        results = title_results;
    } else if (field == FieldType::CONTENT) {
        /* Do nothing */
    }

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

std::vector<int> Indexer::search(const std::vector<std::string> &query_tokens, FieldType field) const {
    /* Stack approach thanks to postfix notation */
    std::vector<std::vector<int>> results;

    /* For each token in the query (in postfix notation) */
    for (const auto &token : query_tokens) {
        /* AND is just intersection */
        if (token == operators_map[Operator::AND]) {
            /* Pop two results from the stack */
            auto result_2 = results.back();
            results.pop_back();
            auto result_1 = results.back();
            results.pop_back();

            /* Intersect */
            auto result = std::vector<int>();
            std::set_intersection(result_1.begin(), result_1.end(), result_2.begin(), result_2.end(), std::back_inserter(result));

            /* Push the result back to the stack */
            results.emplace_back(result);
        /* OR is just union */
        } else if (token == operators_map[Operator::OR]) {
            /* Pop two results from the stack */
            auto result_2 = results.back();
            results.pop_back();
            auto result_1 = results.back();
            results.pop_back();

            /* Union */
            auto result = std::vector<int>();
            std::set_union(result_1.begin(), result_1.end(), result_2.begin(), result_2.end(), std::back_inserter(result));

            /* Push the result back to the stack */
            results.emplace_back(result);
        /* NOT is harder, but not really */
        } else if (token == operators_map[Operator::NOT]) {
            /* One operand this time */
            auto result = results.back();
            results.pop_back();

            /* NOT */
            auto not_result = std::vector<int>();
            for (auto &doc : this->collection)
                if (std::find(result.begin(), result.end(), doc.id) == result.end())
                    not_result.emplace_back(doc.id);

            /* Push the result back to the stack */
            results.emplace_back(not_result);
        /* Just a word */
        } else {
            /* If the word is in the index, push the result to the stack */
            /* Also use this only for ALL and CONTENT fields => not for TITLE */
            if (this->index.find(token) != this->index.end() && field != FieldType::TITLE) {
                auto result = std::vector<int>();
                for (const auto &[doc_id, _] : this->index.at(token).doc_tf_idf)
                    result.emplace_back(doc_id);
                results.emplace_back(result);
                continue;
            /* If the word is in the title index, push the result to the stack */
            /* Also use this only for ALL and TITLE fields => not for CONTENT */
            } else if (this->title_index.find(token) != this->title_index.end() && field != FieldType::CONTENT) {
                auto result = std::vector<int>();
                for (const auto &[doc_id, _]: this->title_index.at(token).doc_tf_idf)
                    result.emplace_back(doc_id);
                results.emplace_back(result);
            /* If the word is not in the index, push an empty result to the stack */
            } else {
                results.emplace_back();
            }
        }
    }

    return results.back();
}

json Indexer::to_json() const {
    json j;
    j["collection"] = json::array();
    for (const auto &doc : this->collection)
        j["collection"].push_back(doc.to_json());
    j["doc_cache"] = json::array();
    for (const auto &[id, doc] : this->doc_cache)
        j["doc_cache"].push_back(doc.to_json());
    j["index"] = json::object();
    for (const auto &[word, element] : this->index)
        j["index"][word] = element.to_json();
    j["title_index"] = json::object();
    for (const auto &[word, element] : this->title_index)
        j["title_index"][word] = element.to_json();
    j["norms"] = this->norms;
    j["title_norms"] = this->title_norms;
    return j;
}

void Indexer::from_json(const json &j) {
    auto temp = j.at("collection");
    for (const auto &doc : temp) {
        TokenizedDocument temp_doc;
        temp_doc.from_json(doc);
        this->collection.emplace_back(temp_doc);
    }
    temp = j.at("doc_cache");
    for (const auto &doc : temp) {
        Document temp_doc;
        temp_doc.from_json(doc);
        this->doc_cache.insert({temp_doc.id, temp_doc});
    }
    temp = j.at("index");
    for (const auto &element : temp.items())
        this->index.insert({element.key(), map_element::from_json(element.value())});
    temp = j.at("title_index");
    for (const auto &element : temp.items())
        this->title_index.insert({element.key(), map_element::from_json(element.value())});
    this->norms = j.at("norms").get<std::map<int, float>>();
    this->title_norms = j.at("title_norms").get<std::map<int, float>>();
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
