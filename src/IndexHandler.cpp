#include "IndexHandler.h"

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

std::vector<TokenizedDocument> IndexHandler::preprocess_documents(Preprocessor &preprocessor, std::vector<Document> &docs, bool verbose) {
    if (verbose)
        std::cout << "Preprocessing documents..." << std::endl;
    auto t_start = std::chrono::high_resolution_clock::now();

    auto tokenized_docs = std::vector<TokenizedDocument>();
    for (auto &doc : docs)
        tokenized_docs.emplace_back(
                doc.id,
                preprocessor.preprocess_text(doc.title, true, false),
                preprocessor.preprocess_text(doc.toc, true, false),
                preprocessor.preprocess_text(doc.h1, true, false),
                preprocessor.preprocess_text(doc.h2, true, false),
                preprocessor.preprocess_text(doc.h3, true, false),
                preprocessor.preprocess_text(doc.content, true, true)
        );

    auto t_end = std::chrono::high_resolution_clock::now();
    if (verbose) {
        std::cout << "Preprocessed " << tokenized_docs.size() << " documents" << std::endl;
        std::cout << "Preprocessing done in " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count() << "ms" << std::endl << std::endl;
    }

    return tokenized_docs;
}

void IndexHandler::add_docs(Indexer &indexer, Preprocessor &preprocessor, std::vector<Document> &docs, bool verbose) {
    if (verbose) {
        std::cout << "Adding new documents..." << std::endl << "IDs: ";
        for (auto &doc : docs)
            std::cout << doc.id << ", ";
        std::cout << std::endl;
    }

    auto tokenized_docs = preprocess_documents(preprocessor, docs, false);

    indexer.add_docs(docs, tokenized_docs);
}

std::vector<Document> IndexHandler::get_docs(Indexer &indexer, std::vector<int> &doc_ids, bool verbose) {
    if (verbose) {
        std::cout << "Getting documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    auto result = indexer.get_docs(doc_ids);

    return result;
}

void IndexHandler::update_docs(Indexer &indexer, Preprocessor &preprocessor, std::vector<int> &doc_ids, std::vector<Document> &docs, bool verbose) {
    if (verbose) {
        std::cout << "Updating documents..."  << std::endl << "IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    auto tokenized_docs = preprocess_documents(preprocessor, docs, false);

    indexer.update_docs(doc_ids, docs, tokenized_docs);
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

void IndexHandler::print_query_results(const std::string &query, const std::pair<std::vector<Document>, std::vector<float>> &result) {
    std::cout << "Top " << result.first.size() << " search results for \"" << query << "\":" << std::endl;
    for (auto i = 0; i < result.first.size(); i++)
        std::cout << "Rank: " << i + 1 << ", ID: " << result.first[i].id << ", Title: " << result.first[i].title << ", Score: " << result.second[i] << std::endl;
    std::cout << std::endl;
}

void IndexHandler::print_query_results(const std::string &query, std::vector<Document> &result) {
    std::cout << "Documents that contain \"" << query << "\":" << std::endl;
    std::cout << "Found " << result.size() << " documents" << std::endl << "Results:" << std::endl;
    for (auto &doc : result)
        std::cout << "ID: " << doc.id << ", Title: " << doc.title << std::endl;
    std::cout << std::endl;
}

std::pair<std::vector<Document>, std::vector<float>> IndexHandler::search(Indexer &indexer, Preprocessor &preprocessor, std::string &query, int k, bool print) {
    auto query_tokens = preprocessor.preprocess_text(query, true, false);

    auto result = indexer.search(query_tokens, k);

    auto result_docs = get_docs(indexer, result.first, false);

    if (print)
        print_query_results(query, {result_docs, result.second});

    return {result_docs, result.second};
}

std::vector<Document> IndexHandler::search(Indexer &indexer, Preprocessor &preprocessor, std::string &query, bool print) {
    std::cout << "Query: " << query << std::endl << "Postfix notation: ";
    auto bool_tokens = preprocessor.parse_bool_query(query);
    for (auto &token : bool_tokens)
        std::cout << token << " ";
    std::cout << std::endl;

    auto result_ids = indexer.search(bool_tokens);

    auto result_docs = get_docs(indexer, result_ids, false);

    if (print)
        print_query_results(query, result_docs);

    return result_docs;
}
