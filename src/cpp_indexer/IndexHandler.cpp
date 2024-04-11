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

std::vector<TokenizedDocument> IndexHandler::preprocess_documents(std::vector<Document> &docs, bool verbose) {
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
    auto tokenized_doc_vec = preprocess_documents(doc_vec, false);

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

    auto tokenized_docs = preprocess_documents(docs, false);

    indexer.add_docs(docs, tokenized_docs);
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

    auto tokenized_docs = preprocess_documents(docs, false);

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

std::pair<std::vector<Document>, std::vector<float>> IndexHandler::search(Indexer &indexer, std::string &query, int k, bool print) {
    auto query_tokens = preprocessor.preprocess_text(query, true, false);

    auto result = indexer.search(query_tokens, k);

    auto result_docs = get_docs(indexer, result.first, false);

    if (print)
        print_query_results(query, {result_docs, result.second});

    return {result_docs, result.second};
}

std::vector<Document> IndexHandler::search(Indexer &indexer, std::string &query, bool print) {
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


std::vector<std::string> IndexHandler::detect_lang(const std::vector<Document> &docs, bool verbose) {
    if (verbose)
        std::cout << "Detecting language of " << docs.size() << " documents..." << std::endl;

    /* mkdir tmp_detect */
    std::filesystem::create_directory("tmp_detect");

    /* Concatenate the title and content */
    for (auto &doc : docs) {
        std::ofstream file("tmp_detect/" + std::to_string(doc.id) + ".txt");
        file << doc.title << " " << doc.content;
        file.close();
    }

    /* Detect the language */
    auto result = PyHandler::run_lang_detector("tmp_detect/");

    /* Remove the tmp_detect directory */
    std::filesystem::remove_all("tmp_detect");

    /* Find the language in result based on print(f"Predicted language: {labels}") */
    std::string start_str = "Predicted languages: ";
    auto start = result.find(start_str);
    auto lang = result.substr(start + start_str.size());

    if (verbose)
        std::cout << "Detected languages: " << lang << std::endl;

    /* lang = ['cs', 'cs', 'cs', 'cs', 'cs'] */
    auto langs = std::vector<std::string>();
    auto start_idx = 0;
    while (true) {
        auto start = lang.find("'", start_idx);
        if (start == std::string::npos)
            break;
        auto end = lang.find("'", start + 1);
        langs.push_back(lang.substr(start + 1, end - start - 1));
        start_idx = end + 1;
    }

    return langs;
}

std::vector<std::string> IndexHandler::detect_lang(Indexer &indexer, const std::vector<int> &doc_ids, bool verbose) {
    if (verbose) {
        std::cout << "Detecting language of documents with IDs: ";
        for (auto &doc_id : doc_ids)
            std::cout << doc_id << ", ";
        std::cout << std::endl;
    }

    /* Get the documents */
    auto docs = get_docs(indexer, doc_ids, false);

    return detect_lang(docs, verbose);
}
