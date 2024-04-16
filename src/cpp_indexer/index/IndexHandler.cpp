#include "IndexHandler.h"

Preprocessor IndexHandler::preprocessor = Preprocessor();

std::vector<Document> IndexHandler::load_documents(const std::string &dir_path, bool verbose) {
    if (verbose)
        std::cout << "Loading documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();

    auto docs = DataLoader::load_json_documents_from_dir(dir_path);

    auto t_end = std::chrono::high_resolution_clock::now();
    if (verbose) {
        std::cout << "Loaded " << docs.size() << " documents" << std::endl;
        std::cout << "Loading done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
    }

    return docs;
}

std::pair<std::vector<TokenizedDocument>, std::map<std::string, std::map<int, std::vector<int>>>> IndexHandler::preprocess_documents(std::vector<Document> &docs, bool verbose) {
    if (verbose)
        std::cout << "Preprocessing documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();

    auto tokenized_docs = std::vector<TokenizedDocument>();
    std::map<int, std::map<std::string, std::vector<int>>> positions;
    for (auto &doc : docs) {
        auto [title, title_pos] = preprocessor.preprocess_text(doc.title, true, false);
        auto [toc, toc_pos] = preprocessor.preprocess_text(doc.toc, true, false);
        auto [h1, h1_pos] = preprocessor.preprocess_text(doc.h1, true, false);
        auto [h2, h2_pos] = preprocessor.preprocess_text(doc.h2, true, false);
        auto [h3, h3_pos] = preprocessor.preprocess_text(doc.h3, true, false);
        auto [content, content_pos] = preprocessor.preprocess_text(doc.content, true, true);
        tokenized_docs.emplace_back(doc.id, title, toc, h1, h2, h3, content);
        positions[doc.id] = content_pos;
    }

    /* Transform positions to a map of word -> (doc_id, positions) */
    std::map<std::string, std::map<int, std::vector<int>>> positions_map;
    for (const auto& [doc_id, doc_positions] : positions)
        for (const auto& [word, pos] : doc_positions)
            positions_map[word][doc_id] = pos;

    auto t_end = std::chrono::high_resolution_clock::now();
    if (verbose) {
        std::cout << "Preprocessed " << tokenized_docs.size() << " documents" << std::endl;
        std::cout << "Preprocessing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
    }

    return {tokenized_docs, positions_map};
}

void IndexHandler::save_index(Indexer &indexer, const string &index_path) {
    std::cout << "Saving index to " << index_path << "..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();

    DataLoader::save_index_to_file(indexer, index_path);

    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "Index saved in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
}

void IndexHandler::load_index(Indexer &indexer, const string &index_path) {
    std::cout << "Loading index from " << index_path << "..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();

    DataLoader::load_index_from_file(indexer, index_path);

    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "Index loaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
}

void IndexHandler::add_doc_url(Indexer &indexer, const string &url, bool verbose) {
    if (verbose)
        std::cout << "Downloading document from " << url << "..." << std::endl;

    /* Run the crawler to download the document */
    auto result = PyHandler::run_crawler(url);

    /* Find the file in result based on print("Successfully wrote to file: ../download/" + title + ".json") */
    std::string start_str = "Successfully wrote to file: ";
    std::string end_str = ".json";
    auto start = result.find(start_str);
    auto end = result.find(end_str);
    auto file = result.substr(start + start_str.size(), end - start - start_str.size() + end_str.size());

    if (verbose)
        std::cout << "Document downloaded and saved to: " << file << std::endl;

    /* Load and preprocess the downloaded document */
    auto doc = DataLoader::load_json_document(file);
    auto doc_vec = std::vector<Document>{doc};

    /* Add the document to the indexer */
    add_docs(indexer, doc_vec, verbose);
}

void IndexHandler::add_docs(Indexer &indexer, std::vector<Document> &docs, bool verbose) {
    if (verbose) {
        std::cout << "Adding new documents..." << std::endl << "IDs: ";
        for (auto &doc : docs)
            std::cout << doc.id << ", ";
        std::cout << std::endl;
    }

    auto [tokenized_docs, positions] = preprocess_documents(docs, false);

    indexer.add_docs(docs, tokenized_docs, positions);
}

std::vector<Document> IndexHandler::get_docs(Indexer &indexer, const std::vector<int> &doc_ids, bool verbose) {
    if (verbose) {
        std::cout << "Getting documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    auto result = indexer.get_docs(doc_ids);

    return result;
}

void IndexHandler::update_docs(Indexer &indexer, std::vector<int> &doc_ids, std::vector<Document> &docs, bool verbose) {
    if (verbose) {
        std::cout << "Updating documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    auto [tokenized_docs, positions] = preprocess_documents(docs, false);

    indexer.update_docs(doc_ids, docs, tokenized_docs, positions);
}

void IndexHandler::remove_docs(Indexer &indexer, std::vector<int> &doc_ids, bool verbose) {
    if (verbose) {
        std::cout << "Removing documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    indexer.remove_docs(doc_ids);
}

void IndexHandler::print_query_results(const std::string &query, const std::pair<std::vector<Document>, std::vector<float>> &result, std::map<std::string, std::map<int, std::vector<int>>> &positions) {
    std::cout << "Top " << result.first.size() << " search results for \"" << query << "\":" << std::endl;
    for (auto i = 0; i < result.first.size(); i++) {
        std::cout << "Rank: " << i + 1 << ", ID: " << result.first[i].id << ", Title: " << result.first[i].title << ", Score: " << result.second[i] << std::endl;
        /* Commented out because it makes messy output */
//        for (const auto &[word, pos] : positions) {
//            for (const auto &[doc_id, positions_map] : pos)
//                if (doc_id == result.first[i].id) {
//                    std::cout << "Word: " << word << ", Positions: ";
//                    for (const auto &p : positions_map)
//                        std::cout << p << ", ";
//                    std::cout << std::endl;
//                }
//        }
    }
    std::cout << std::endl;
}

void IndexHandler::print_query_results(const std::string &query, std::vector<Document> &result, std::map<std::string, std::map<int, std::vector<int>>> &positions) {
    std::cout << "Documents that contain \"" << query << "\":" << std::endl;
    std::cout << "Found " << result.size() << " documents" << std::endl << "Results:" << std::endl;
    for (auto &doc : result) {
        std::cout << "ID: " << doc.id << ", Title: " << doc.title << std::endl;
        /* Commented out because it makes messy output */
//        for (const auto &[word, pos] : positions) {
//            for (const auto &[doc_id, positions_map] : pos)
//                if (doc_id == doc.id) {
//                    std::cout << "Word: " << word << ", Positions: ";
//                    for (const auto &p : positions_map)
//                        std::cout << p << ", ";
//                    std::cout << std::endl;
//                }
//        }
    }
    std::cout << std::endl;
}

std::tuple<std::vector<Document>, std::vector<float>, std::map<std::string, std::map<int, std::vector<int>>>> IndexHandler::search(Indexer &indexer, std::string &query, int k, FieldType field, bool print) {
    auto [query_tokens, _] = preprocessor.preprocess_text(query, true, false);

    auto [doc_ids, scores, positions] = indexer.search(query_tokens, k, field);

    auto result_docs = get_docs(indexer, doc_ids, false);

    if (print)
        print_query_results(query, {result_docs, scores}, positions);

    return {result_docs, scores, positions};
}

std::tuple<std::vector<Document>, std::map<std::string, std::map<int, std::vector<int>>>> IndexHandler::search(Indexer &indexer, std::string &query, FieldType field, bool print) {
    std::cout << "Query: " << query << std::endl << "Postfix notation: ";
    auto bool_tokens = preprocessor.parse_bool_query(query);
    for (auto &token : bool_tokens)
        std::cout << token << " ";
    std::cout << std::endl;

    auto [result_ids, positions] = indexer.search(bool_tokens, field);

    auto result_docs = get_docs(indexer, result_ids, false);

    if (print)
        print_query_results(query, result_docs, positions);

    return {result_docs, positions};
}

std::tuple<std::string, std::vector<int>> IndexHandler::create_snippet(Indexer &indexer, int doc_id, std::map<std::string, std::map<int, vector<int>>> &positions, int window_size) {
    auto doc = indexer.get_doc(doc_id);
    auto content = doc.content;
    auto tokenized_doc = indexer.get_tokenized_doc(doc_id);
    auto words = tokenized_doc.content;

    if (words.size() <= window_size) /* WTF? ID 278 has only 28 words */
        window_size = words.size();

    std::tuple<int, int, int> best_window = std::make_tuple(0, window_size - 1, 0); // start, end, unique words count

    /* Positions = word -> (doc_id, positions) */
    for (const auto &[word, pos_list] : positions) {
        for (const auto &[doc_id_, positions_] : pos_list) {
            if (doc_id_ != doc_id)
                continue;
            for (const auto &pos : positions_) {
                /* Find the window around the position */
                int start = std::max(0, pos - window_size / 2);
                int end = std::min((int)words.size() - 1, start + window_size - 1);
                start = std::max(0, end - window_size + 1);

                std::set<std::string> unique_words_in_window;
                for (int i = start; i <= end; i++)
                    if (positions.count(words[i]) > 0)
                        unique_words_in_window.insert(words[i]);

                if (unique_words_in_window.size() > std::get<2>(best_window))
                    best_window = std::make_tuple(start, end, unique_words_in_window.size());
            }
        }
    }

    /* Which words in the window should be highlighted */
    std::vector<int> highlight_indexes{};
    for (int i = std::get<0>(best_window); i <= std::get<1>(best_window); i++)
        if (positions.count(words[i]) > 0)
            highlight_indexes.push_back(i - std::get<0>(best_window));

    std::vector<std::string> words_vec;
    std::istringstream iss(content);
    for (std::string s; iss >> s; )
        words_vec.push_back(s);

    std::string snippet;
    for (int i = std::get<0>(best_window); i <= std::get<1>(best_window); i++)
        snippet += words_vec[i] + " ";

    return {snippet, highlight_indexes};
}
