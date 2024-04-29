#include "GUI.h"
#include "FileBasedLoader.h"

/** File based index */
bool FILE_BASED = false;
/** Language detection is REALLY sloooow */
bool DETECT_LANG = true;
/** Use lemmatization or stemming */
bool USE_LEMMA = true;

/**
 * Parse arguments
 * @param argc Argument count
 * @param argv Argument values
 */
void parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--help") {
            std::cout << "Usage: ./cpp_indexer [--file-based] [--no-lang-detect] [--lemma | --stem]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "\t--file-based\t\tUse file based index" << std::endl;
            std::cout << "\t--no-lang-detect\tDo not detect language" << std::endl;
            std::cout << "\t--lemma\t\t\t\tUse lemmatization" << std::endl;
            std::cout << "\t--stem\t\t\t\tUse stemming" << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (std::string(argv[i]) == "--file-based")
            FILE_BASED = true;
        if (std::string(argv[i]) == "--no-lang-detect")
            DETECT_LANG = false;
        if (std::string(argv[i]) == "--lemma")
            USE_LEMMA = true;
        if (std::string(argv[i]) == "--stem")
            USE_LEMMA = false;
    }
}

/**
 * Main function
 * @return Exit code
 */
int main(int argc, char **argv) {
    /* Parse arguments, set FILE_BASED and DETECT_LANG */
    parse_args(argc, argv);

    /* Run the GUI */
    GUI gui = GUI();
    gui.run();

//    auto docs = IndexHandler::load_documents("../data");
//    auto [tokenized_docs, positions_map] = IndexHandler::preprocess_documents(docs);
//    auto indexer = Indexer(docs, tokenized_docs, positions_map);
//
//    IndexHandler::save_index(indexer, "../index/index1.json");
//    auto indexer = Indexer();
//    IndexHandler::load_index(indexer, "../index/index1.json");
//
//    std::string query = "Geralt of Rivia";
//    auto [docs_result, scores, positions] = IndexHandler::search(indexer, query, 3, FieldType::ALL);
//    auto positions_geralt = positions["geralt"][954];
//    for (const auto &pos : positions_geralt)
//        std::cout << pos << " ";
//    std::cout << std::endl;
//
//    /* Open Magistrovo hlaseni.json */
//    std::string doc_path = "../data/Magistrovo hlaseni.json";
//    auto doc = DataLoader::load_json_document(doc_path);
//    Preprocessor preprocessor = Preprocessor();
//    auto [tokens, _] = preprocessor.tokenize(doc.content);
//
//    std::cout << tokens[237] << std::endl;
//
//    /* Run the GUI */
//    GUI gui = GUI();
//    gui.run();

//    std::string base_dir = FILE_BASED_INDEX_PATH + "index1/";
//    if (!std::filesystem::exists(base_dir))
//        std::filesystem::create_directory(base_dir);
//
//    /* Load documents */
//    {
//        auto docs = IndexHandler::load_documents("../data");
//        FileBasedLoader::save_doc_cache(docs, base_dir);
//    }
//
//    /* Preprocess documents */
//    {
//        std::vector<Document> docs;
//        {
//            auto doc_cache = FileBasedLoader::load_doc_cache(base_dir);
//            for (const auto &[id, doc]: doc_cache)
//                docs.push_back(doc);
//        }
//        auto [tokenized_docs, positions_map] = IndexHandler::preprocess_documents(docs);
//        FileBasedLoader::save_tokenized_docs(tokenized_docs, base_dir);
//        FileBasedLoader::save_positions_map(positions_map, base_dir);
//    }
//
//    /* Index documents */
//    auto indexer = Indexer(base_dir);
//
//    /* Save the index */
//    IndexHandler::save_index(indexer, "../index/index1.json");
//
//    /* Load the index */
//    indexer = Indexer();
//    IndexHandler::load_index(indexer, "../index/index1.json");
//
//    /* Search "Geralt z Rivie" */
//    std::string query = "Geralt Rivie";
//    auto [docs_result, scores, positions] = IndexHandler::search(indexer, query, 3, FieldType::ALL, 3);
//    auto [snippet, highlight_index] = IndexHandler::create_snippet(indexer, docs_result[0].id, positions, 30);
//    std::cout << snippet << std::endl;
//    for (const auto &index : highlight_index)
//        std::cout << index << " ";
//    std::cout << std::endl << std::endl;
//    std::tie(docs_result, scores, positions) = IndexHandler::search(indexer, query, 3, FieldType::TITLE);
//
//    /* Update the document, so it contains "Geralt z Rivie" more and so it is way more relevant (see Score print) */
//    std::vector<int> new_doc_ids = {550};
//    std::vector<Document> new_docs = {{550, {"už ne Geralt z Rivie"}, {}, {}, {}, {}, {"Geralt z Rivie Geralt z Rivie Geralt z Rivie Geralt z Rivie"}}};
//    IndexHandler::update_docs(indexer, new_doc_ids, new_docs);
//
//    /* Search "Geralt z Rivie" again */
//    result = IndexHandler::search(indexer, query, 3);
//
//    /* Remove the document completely now and see how the score of others changes too, because overall IDF changes */
//    IndexHandler::remove_docs(indexer, new_doc_ids);
//
//    /* Search "Geralt z Rivie" again and see how the score of others changes (also ID 550 is gone, shocking!) */
//    result = IndexHandler::search(indexer, query, 3);
//
//    /* Search "Geralt z Rivie" boolean */
//    query = "NOT Geralt AND (z OR NOT NOT Rivie)";
//    auto result_bool = IndexHandler::search(indexer, query);
//    result_bool = IndexHandler::search(indexer, query, FieldType::TITLE);
//
//    /* Download https://zaklinac.fandom.com/wiki/Geralt_z_Rivie */
//    IndexHandler::add_doc_url(indexer, "https://zaklinac.fandom.com/wiki/Geralt_z_Rivie");

    return EXIT_SUCCESS;
}
