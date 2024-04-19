#include "GUI.h"
#include "FileBasedLoader.h"

/** File based index */
bool FILE_BASED = false;
/** Language detection is REALLY sloooow */
bool DETECT_LANG = true;

/**
 * Main function
 * @return Exit code
 */
int main() {
    GUI gui = GUI();
    gui.run();

    std::string base_dir = FILE_BASED_INDEX_PATH + "index1/";
    if (!std::filesystem::exists(base_dir))
        std::filesystem::create_directory(base_dir);

    /* Load documents */
//    {
//        auto docs = IndexHandler::load_documents("../data");
//        FileBasedLoader::save_doc_cache(docs, base_dir);
//    }

    /* Preprocess documents */
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

    /* Index documents */
    auto indexer = Indexer(base_dir);

//    /* Save the index */
//    IndexHandler::save_index(indexer, "../index/index1.json");
//
//    /* Load the index */
//    indexer = Indexer();
//    IndexHandler::load_index(indexer, "../index/index1.json");
//
    /* Search "Geralt z Rivie" */
    std::string query = "Geralt Rivie";
    auto [docs_result, scores, positions] = IndexHandler::search(indexer, query, 3, FieldType::ALL, 3);
    auto [snippet, highlight_index] = IndexHandler::create_snippet(indexer, docs_result[0].id, positions, 30);
    std::cout << snippet << std::endl;
    for (const auto &index : highlight_index)
        std::cout << index << " ";
    std::cout << std::endl << std::endl;
    std::tie(docs_result, scores, positions) = IndexHandler::search(indexer, query, 3, FieldType::TITLE);

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
