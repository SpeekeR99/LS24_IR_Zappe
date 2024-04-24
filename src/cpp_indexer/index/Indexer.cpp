#include "Indexer.h"

#include <utility>

Indexer::Indexer() : collection(std::vector<TokenizedDocument>()), keywords(), doc_cache(), index(std::map<std::string, map_element>()), norms(std::map<int, float>()), positions_map() {
    /* Nothing to do here :) */
}

Indexer::Indexer(const string &index_path_dir, bool reindex_immediately) : collection(std::vector<TokenizedDocument>()), keywords(), doc_cache(), index(std::map<std::string, map_element>()), norms(std::map<int, float>()), positions_map() {
    this->index_path_dir = index_path_dir;
    if (reindex_immediately)
        this->index_everything_file_based();
}

Indexer::Indexer(const std::vector<Document> &original_collection, const std::vector<TokenizedDocument> &tokenized_collection, std::map<std::string, std::map<int, std::vector<int>>> &positions_map) : collection(std::vector<TokenizedDocument>()), keywords(), doc_cache(), index(std::map<std::string, map_element>()), norms(std::map<int, float>()), positions_map() {
    this->add_docs(original_collection, tokenized_collection, positions_map);
}

void Indexer::docs_to_keywords() {
    /* Clear the keywords */
    this->keywords.clear();
    /* Add words from the collection to the keywords */
    std::vector<TokenizedDocument> collection_to_use;
    if (FILE_BASED)
        collection_to_use = FileBasedLoader::load_tokenized_docs(this->index_path_dir);
    else
        collection_to_use = this->collection;
    for (const auto &doc : collection_to_use) {
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

void Indexer::index_everything_file_based() {
    std::cout << "Indexing documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();

    if (DETECT_LANG) {
        auto doc_cache_ = FileBasedLoader::load_doc_cache(this->index_path_dir);
        std::vector<Document> docs;
        for (const auto &[_, doc]: doc_cache_)
            docs.emplace_back(doc);
        auto langs = PyHandler::detect_lang(docs);
        auto docs_tok = FileBasedLoader::load_tokenized_docs(this->index_path_dir);
        for (auto i = 0; i < docs.size(); i++) {
            auto id = docs[i].id;
            doc_cache_[id].lang = langs[i];
            docs_tok[i].lang = langs[i];
        }
        FileBasedLoader::save_doc_cache(doc_cache_, this->index_path_dir);
        FileBasedLoader::save_tokenized_docs(docs_tok, this->index_path_dir);
    }

    this->docs_to_keywords();

    TF_IDF::calc_tf_idf_file_based(this->index_path_dir);
    TF_IDF::calc_tf_idf_file_based(this->index_path_dir, true);

    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "Indexing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
}

void Indexer::add_docs(const std::vector<Document> &docs, const std::vector<TokenizedDocument> &tokenized_docs, std::map<std::string, std::map<int, std::vector<int>>> &positions_map) {
    if (FILE_BASED) {
        auto doc_cache_ = FileBasedLoader::load_doc_cache(this->index_path_dir);
        auto tokenized_docs_ = FileBasedLoader::load_tokenized_docs(this->index_path_dir);
        auto positions_map_ = FileBasedLoader::load_positions_map(this->index_path_dir);
        for (int i = 0; i < docs.size(); i++) {
            doc_cache_.erase(docs[i].id);
            doc_cache_.insert({docs[i].id, docs[i]});
            tokenized_docs_[i] = tokenized_docs[i];
        }
        positions_map_ = std::move(positions_map);
        FileBasedLoader::save_doc_cache(doc_cache_, this->index_path_dir);
        FileBasedLoader::save_tokenized_docs(tokenized_docs_, this->index_path_dir);
        FileBasedLoader::save_positions_map(positions_map_, this->index_path_dir);
        this->index_everything_file_based();
    } else {
        for (int i = 0; i < docs.size(); i++) {
            this->doc_cache.insert({docs[i].id, docs[i]});
            this->collection.emplace_back(tokenized_docs[i]);
        }
        this->positions_map = std::move(positions_map);
        this->index_everything();
    }
}

Document Indexer::get_doc(int doc_id) {
    std::unordered_map<int, Document> doc_cache_to_use;
    if (FILE_BASED)
        doc_cache_to_use = FileBasedLoader::load_doc_cache(this->index_path_dir);
    else
        doc_cache_to_use = this->doc_cache;
    try {
        return doc_cache_to_use.at(doc_id);
    } catch (const std::out_of_range &e) {
        std::cerr << "[ERROR]: Document with ID " << doc_id << " not found!" << std::endl;
        return {-1, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}};
    }
}

TokenizedDocument Indexer::get_tokenized_doc(int doc_id) {
    std::vector<TokenizedDocument> collection_to_use;
    if (FILE_BASED)
        collection_to_use = FileBasedLoader::load_tokenized_docs(this->index_path_dir);
    else
        collection_to_use = this->collection;
    for (const auto &doc : collection_to_use)
        if (doc.id == doc_id)
            return doc;
    return {-1, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}, {"NENALEZENO"}};
}

std::vector<Document> Indexer::get_docs(const std::vector<int> &doc_ids) {
    std::vector<Document> result;
    result.reserve(doc_ids.size());
    for (const auto &doc_id : doc_ids)
        result.emplace_back(this->get_doc(doc_id));
    return result;
}

void Indexer::update_docs(const std::vector<int> &doc_ids, const std::vector<Document> &docs, const std::vector<TokenizedDocument> &tokenized_docs, std::map<std::string, std::map<int, std::vector<int>>> &positions_map) {
    if (FILE_BASED) {
        auto doc_cache_ = FileBasedLoader::load_doc_cache(this->index_path_dir);
        auto tokenized_docs_ = FileBasedLoader::load_tokenized_docs(this->index_path_dir);
        auto positions_map_ = FileBasedLoader::load_positions_map(this->index_path_dir);
        for (int i = 0; i < doc_ids.size(); i++) {
            doc_cache_.erase(doc_ids[i]);
            doc_cache_.insert({doc_ids[i], docs[i]});
            tokenized_docs_[i] = tokenized_docs[i];
        }
        positions_map_ = std::move(positions_map);
        FileBasedLoader::save_doc_cache(doc_cache_, this->index_path_dir);
        FileBasedLoader::save_tokenized_docs(tokenized_docs_, this->index_path_dir);
        FileBasedLoader::save_positions_map(positions_map_, this->index_path_dir);
        this->index_everything_file_based();
    } else {
        for (int i = 0; i < doc_ids.size(); i++) {
            this->doc_cache.erase(doc_ids[i]);
            this->doc_cache.insert({doc_ids[i], docs[i]});

            for (auto &d: this->collection)
                if (d.id == doc_ids[i])
                    d = tokenized_docs[i];
        }
        this->positions_map = std::move(positions_map);
        this->index_everything();
    }
}

void Indexer::remove_docs(const std::vector<int> &doc_ids) {
    if (FILE_BASED) {
        auto doc_cache_ = FileBasedLoader::load_doc_cache(this->index_path_dir);
        auto tokenized_docs_ = FileBasedLoader::load_tokenized_docs(this->index_path_dir);
        auto positions_map_ = FileBasedLoader::load_positions_map(this->index_path_dir);
        for (const auto &doc_id : doc_ids) {
            try {
                doc_cache_.at(doc_id);
            } catch (const std::out_of_range &e) {
                std::cerr << "[ERROR]: Document with ID " << doc_id << " not found!" << std::endl;
                continue;
            }
            for (auto it = tokenized_docs_.begin(); it != tokenized_docs_.end(); it++)
                if (it->id == doc_id) {
                    tokenized_docs_.erase(it);
                    doc_cache_.erase(doc_id);
                    break;
                }
            for (auto it = positions_map_.begin(); it != positions_map_.end(); it++)
                if (it->second.begin()->first == doc_id) {
                    positions_map_.erase(it);
                    break;
                }
        }
        FileBasedLoader::save_doc_cache(doc_cache_, this->index_path_dir);
        FileBasedLoader::save_tokenized_docs(tokenized_docs_, this->index_path_dir);
        FileBasedLoader::save_positions_map(positions_map_, this->index_path_dir);
        this->index_everything_file_based();
    } else {
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
                    this->doc_cache.erase(doc_id);
                    break;
                }
            for (auto it = this->positions_map.begin(); it != this->positions_map.end(); it++)
                if (it->second.begin()->first == doc_id) {
                    this->positions_map.erase(it);
                    break;
                }
        }
        this->index_everything();
    }
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

float Indexer::cosine_similarity_file_based(const map<std::string, float> &query, int doc_id, std::map<int, float> &norms, std::map<std::string, map_element> &index) const {
    /* Initialize dot product and norms */
    float dot = 0;
    float norm_query = 0;
    float norm_doc = norms.at(doc_id);

    /* Iterate over TF-IDF of words in query */
    for (const auto& [word, value] : query) {
        /* If word is found in a document use it for dot product */
        if (index.find(word) != index.end()) {
            for (const auto &[id, tf_idf]: index.at(word).doc_tf_idf)
                if (id == doc_id)
                    dot += value * tf_idf;
        }
        norm_query += value * value;
    }

    /* Calculate cosine similarity */
    return dot / (std::sqrt(norm_query) * norm_doc);
}

std::tuple<std::vector<int>, std::vector<float>, std::map<std::string, std::map<int, std::vector<int>>>> Indexer::search(const std::vector<std::string> &query, int k, FieldType field, int proximity) const {
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

    /* Take care of nans */
    for (auto& [doc_id, value] : results)
        if (std::isnan(value))
            value = 0;

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

    /* Get positions of the words in the query */
    std::map<std::string, std::map<int, std::vector<int>>> positions;
    for (const auto& word : query) {
        if (this->positions_map.find(word) != this->positions_map.end())
            positions[word] = this->positions_map.at(word);
    }

    /* Postfilter results using proximity search */
    if (proximity > 0) {
        std::map<int, float> filtered_results_ids_prox_score;

        /* For each pair of query words */
        for (int i = 0; i < query.size(); i++) {
            for (int j = i + 1; j < query.size(); j++) {
                // Get the positions of the words in the documents */
                auto &positions1 = positions[query[i]];
                auto &positions2 = positions[query[j]];

                /* For each document where both words appear */
                for (auto &[doc_id, pos1]: positions1) {
                    if (positions2.find(doc_id) != positions2.end()) {
                        auto &pos2 = positions2[doc_id];

                        /* For each pair of positions, calculate the distance */
                        for (int pos_1: pos1) {
                            for (int pos_2: pos2) {
                                int distance = std::abs(pos_1 - pos_2);

                                /* If the distance is less than or equal to the proximity, add the document to the result set */
                                if (distance <= proximity) {
                                    if (filtered_results_ids_prox_score.find(doc_id) == filtered_results_ids_prox_score.end())
                                        filtered_results_ids_prox_score[doc_id] = 0;
                                    filtered_results_ids_prox_score[doc_id] += 1.0 / (1 + distance);
                                }
                            }
                        }
                    }
                }
            }
        }
        /* Replace the original results with the filtered results */
        std::vector<std::pair<int, float>> filtered_results;
        for (const auto& [doc_id, value]: results)
            if (filtered_results_ids_prox_score.find(doc_id) != filtered_results_ids_prox_score.end())
                filtered_results.emplace_back(doc_id, value + filtered_results_ids_prox_score[doc_id]);
        results = filtered_results;
    }

    /* Sort results by cosine similarity */
    std::sort(results.begin(), results.end(), [](const std::pair<int, float> &a, const std::pair<int, float> &b) {
        return a.second > b.second;
    });

    /* Return top k results */
    std::tuple<std::vector<int>, std::vector<float>, std::vector<int>> top_k;
    if (k > results.size())
        k = static_cast<int>(results.size());

    for (int i = 0; i < k; i++) {
        std::get<0>(top_k).emplace_back(results[i].first);
        std::get<1>(top_k).emplace_back(results[i].second);
    }

    /* Filter positions to only include the top k results */
    for (const auto& [word, pos] : positions) {
        std::map<int, std::vector<int>> temp;
        for (const auto& [doc_id, positions_] : pos)
            if (std::find(std::get<0>(top_k).begin(), std::get<0>(top_k).end(), doc_id) != std::get<0>(top_k).end())
                temp[doc_id] = positions_;
        positions[word] = temp;
    }

    return {std::get<0>(top_k), std::get<1>(top_k), positions};
}

std::tuple<std::vector<int>, std::map<std::string, std::map<int, std::vector<int>>>> Indexer::search(const std::vector<std::string> &query_tokens, FieldType field) const {
    /* Stack approach thanks to postfix notation */
    std::vector<std::vector<int>> results;
    std::vector<std::string> query_words;

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
                query_words.emplace_back(token);
                continue;
            /* If the word is in the title index, push the result to the stack */
            /* Also use this only for ALL and TITLE fields => not for CONTENT */
            } else if (this->title_index.find(token) != this->title_index.end() && field != FieldType::CONTENT) {
                auto result = std::vector<int>();
                for (const auto &[doc_id, _]: this->title_index.at(token).doc_tf_idf)
                    result.emplace_back(doc_id);
                results.emplace_back(result);
                query_words.emplace_back(token);
            /* If the word is not in the index, push an empty result to the stack */
            } else {
                results.emplace_back();
            }
        }
    }

    /* Positions of the words in the query */
    std::map<std::string, std::map<int, std::vector<int>>> positions;
    for (const auto& word : query_words) {
        if (this->positions_map.find(word) != this->positions_map.end())
            positions[word] = this->positions_map.at(word);
    }
    /* Filter positions to only include the result documents */
    for (const auto& [word, pos] : positions) {
        std::map<int, std::vector<int>> temp;
        for (const auto& [doc_id, positions_] : pos)
            if (std::find(results.back().begin(), results.back().end(), doc_id) != results.back().end())
                temp[doc_id] = positions_;
        positions[word] = temp;
    }

    return {results.back(), positions};
}

std::tuple<std::vector<int>, std::vector<float>, std::map<std::string, std::map<int, std::vector<int>>>> Indexer::search_file_based(const vector<std::string> &query, int k, FieldType field, int proximity) const {
    /* Calculate TF for the query */
    auto tf_query = TF_IDF::calc_tf(query);
    std::map<std::string, float> tf_idf_query;

    /* Calculate TF-IDF for the query */
    {
        auto index_ = FileBasedLoader::load_tf_idf(index_path_dir);
        for (const auto& [word, value] : tf_query)
            if (index_.find(word) != index_.end())
                tf_idf_query[word] = value * index_.at(word).idf;
            else
                tf_idf_query[word] = 0;
    }

    /* Calculate cosine similarity for each document */
    std::vector<std::pair<int, float>> title_results;
    std::vector<std::pair<int, float>> results;
    {
        auto tokenized_docs = FileBasedLoader::load_tokenized_docs(index_path_dir);
        {
            auto norms = FileBasedLoader::load_tf_idf_norms(index_path_dir);
            auto index = FileBasedLoader::load_tf_idf(index_path_dir);
            for (const auto& doc: tokenized_docs)
                results.emplace_back(doc.id, this->cosine_similarity_file_based(tf_idf_query, doc.id, norms, index));
        }
        {
            auto norms = FileBasedLoader::load_tf_idf_norms(index_path_dir, true);
            auto index = FileBasedLoader::load_tf_idf(index_path_dir, true);
            for (const auto& doc: tokenized_docs)
                title_results.emplace_back(doc.id, this->cosine_similarity_file_based(tf_idf_query, doc.id, norms, index));
        }
    }

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

    /* Take care of nans */
    for (auto& [doc_id, value] : results)
        if (std::isnan(value))
            value = 0;

    /* Get positions of the words in the query */
    std::map<std::string, std::map<int, std::vector<int>>> positions;
    {
        auto positions_map_ = FileBasedLoader::load_positions_map(index_path_dir);
        for (const auto& word : query) {
            if (positions_map_.find(word) != positions_map_.end())
                positions[word] = positions_map_.at(word);
        }
    }

    /* Postfilter results using proximity search */
    if (proximity > 0) {
        std::map<int, float> filtered_results_ids_prox_score;

        /* For each pair of query words */
        for (int i = 0; i < query.size(); i++) {
            for (int j = i + 1; j < query.size(); j++) {
                // Get the positions of the words in the documents */
                auto &positions1 = positions[query[i]];
                auto &positions2 = positions[query[j]];

                /* For each document where both words appear */
                for (auto &[doc_id, pos1]: positions1) {
                    if (positions2.find(doc_id) != positions2.end()) {
                        auto &pos2 = positions2[doc_id];

                        /* For each pair of positions, calculate the distance */
                        for (int pos_1: pos1) {
                            for (int pos_2: pos2) {
                                int distance = std::abs(pos_1 - pos_2);

                                /* If the distance is less than or equal to the proximity, add the document to the result set */
                                if (distance <= proximity) {
                                    if (filtered_results_ids_prox_score.find(doc_id) == filtered_results_ids_prox_score.end())
                                        filtered_results_ids_prox_score[doc_id] = 0;
                                    filtered_results_ids_prox_score[doc_id] += 1.0 / (1 + distance);
                                }
                            }
                        }
                    }
                }
            }
        }
        /* Replace the original results with the filtered results */
        std::vector<std::pair<int, float>> filtered_results;
        for (const auto& [doc_id, value]: results)
            if (filtered_results_ids_prox_score.find(doc_id) != filtered_results_ids_prox_score.end())
                filtered_results.emplace_back(doc_id, value + filtered_results_ids_prox_score[doc_id]);
        results = filtered_results;
    }

    /* Sort results by cosine similarity */
    std::sort(results.begin(), results.end(), [](const std::pair<int, float> &a, const std::pair<int, float> &b) {
        return a.second > b.second;
    });

    /* Return top k results */
    std::tuple<std::vector<int>, std::vector<float>, std::vector<int>> top_k;
    if (k > results.size())
        k = static_cast<int>(results.size());

    for (int i = 0; i < k; i++) {
        std::get<0>(top_k).emplace_back(results[i].first);
        std::get<1>(top_k).emplace_back(results[i].second);
    }

    /* Filter positions to only include the top k results */
    for (const auto& [word, pos] : positions) {
        std::map<int, std::vector<int>> temp;
        for (const auto& [doc_id, positions_] : pos)
            if (std::find(std::get<0>(top_k).begin(), std::get<0>(top_k).end(), doc_id) != std::get<0>(top_k).end())
                temp[doc_id] = positions_;
        positions[word] = temp;
    }

    return {std::get<0>(top_k), std::get<1>(top_k), positions};
}

std::tuple<std::vector<int>, std::map<std::string, std::map<int, std::vector<int>>>> Indexer::search_file_based(const vector<std::string> &query_tokens, FieldType field) const {
    /* Stack approach thanks to postfix notation */
    std::vector<std::vector<int>> results;
    std::vector<std::string> query_words;

    {
        auto index_ = FileBasedLoader::load_tf_idf(index_path_dir);
        auto title_index_ = FileBasedLoader::load_tf_idf(index_path_dir, true);
        auto tokenized_docs = FileBasedLoader::load_tokenized_docs(index_path_dir);

        /* For each token in the query (in postfix notation) */
        for (const auto &token: query_tokens) {
            /* AND is just intersection */
            if (token == operators_map[Operator::AND]) {
                /* Pop two results from the stack */
                auto result_2 = results.back();
                results.pop_back();
                auto result_1 = results.back();
                results.pop_back();

                /* Intersect */
                auto result = std::vector<int>();
                std::set_intersection(result_1.begin(), result_1.end(), result_2.begin(), result_2.end(),
                                      std::back_inserter(result));

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
                std::set_union(result_1.begin(), result_1.end(), result_2.begin(), result_2.end(),
                               std::back_inserter(result));

                /* Push the result back to the stack */
                results.emplace_back(result);
                /* NOT is harder, but not really */
            } else if (token == operators_map[Operator::NOT]) {
                /* One operand this time */
                auto result = results.back();
                results.pop_back();

                /* NOT */
                auto not_result = std::vector<int>();
                for (auto &doc: tokenized_docs)
                    if (std::find(result.begin(), result.end(), doc.id) == result.end())
                        not_result.emplace_back(doc.id);

                /* Push the result back to the stack */
                results.emplace_back(not_result);
                /* Just a word */
            } else {
                /* If the word is in the index, push the result to the stack */
                /* Also use this only for ALL and CONTENT fields => not for TITLE */
                if (index_.find(token) != index_.end() && field != FieldType::TITLE) {
                    auto result = std::vector<int>();
                    for (const auto &[doc_id, _]: index_.at(token).doc_tf_idf)
                        result.emplace_back(doc_id);
                    results.emplace_back(result);
                    query_words.emplace_back(token);
                    continue;
                    /* If the word is in the title index, push the result to the stack */
                    /* Also use this only for ALL and TITLE fields => not for CONTENT */
                } else if (title_index_.find(token) != title_index_.end() && field != FieldType::CONTENT) {
                    auto result = std::vector<int>();
                    for (const auto &[doc_id, _]: title_index_.at(token).doc_tf_idf)
                        result.emplace_back(doc_id);
                    results.emplace_back(result);
                    query_words.emplace_back(token);
                    /* If the word is not in the index, push an empty result to the stack */
                } else {
                    results.emplace_back();
                }
            }
        }
    }

    /* Positions of the words in the query */
    std::map<std::string, std::map<int, std::vector<int>>> positions;
    {
        auto positions_map_ = FileBasedLoader::load_positions_map(index_path_dir);
        for (const auto& word : query_words) {
            if (positions_map_.find(word) != positions_map_.end())
                positions[word] = positions_map_.at(word);
        }
    }

    /* Filter positions to only include the result documents */
    for (const auto& [word, pos] : positions) {
        std::map<int, std::vector<int>> temp;
        for (const auto& [doc_id, positions_] : pos)
            if (std::find(results.back().begin(), results.back().end(), doc_id) != results.back().end())
                temp[doc_id] = positions_;
        positions[word] = temp;
    }

    return {results.back(), positions};
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
    j["positions_map"] = json::object();
    for (const auto& [word, doc_positions] : this->positions_map) {
        j["positions_map"][word] = json::object();
        for (const auto& [doc_id, positions] : doc_positions) {
            j["positions_map"][word][std::to_string(doc_id)] = json::array();
            for (const auto& pos : positions)
                j["positions_map"][word][std::to_string(doc_id)].push_back(pos);
        }
    }
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
    this->positions_map = std::map<std::string, std::map<int, std::vector<int>>>();
    temp = j.at("positions_map");
    for (const auto& [word, doc_positions] : temp.items()) {
        std::map<int, std::vector<int>> temp_map;
        for (const auto& [doc_id, positions] : doc_positions.items()) {
            std::vector<int> temp_vec;
            for (const auto& pos : positions)
                temp_vec.push_back(pos);
            temp_map[std::stoi(doc_id)] = temp_vec;
        }
        this->positions_map[word] = temp_map;
    }
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

std::unordered_set<std::string> Indexer::get_keywords() const {
    return this->keywords;
}

int Indexer::get_max_doc_id() const {
    int max_id = 0;
    for (const auto &doc : this->collection)
        if (doc.id > max_id)
            max_id = doc.id;
    return max_id;
}
