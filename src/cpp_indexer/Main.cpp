#include "GUI.h"

/**
 * Main function
 * @return Exit code
 */
int main() {
//    GUI gui = GUI();
//    gui.run();

    /* Load documents */
    {
        auto docs = IndexHandler::load_documents("../data");
        std::ofstream output("../index/index1_doc_cache.json");
        json j;
        for (const auto &doc : docs)
            j[std::to_string(doc.id)] = doc.to_json();
        output << j.dump(1);
        output.close();
    }

    /* Preprocess documents */
    {
        std::ifstream input("../index/index1_doc_cache.json");
        json j;
        input >> j;
        input.close();
        std::vector<Document> docs;
        for (const auto &item : j.items()) {
            Document temp = Document();
            temp.from_json(item.value());
            docs.push_back(temp);
        }

        auto [tokenized_docs, positions_map] = IndexHandler::preprocess_documents(docs);

        std::ofstream output("../index/index1_tokenized_docs.json");
        json j2;
        for (const auto &doc : tokenized_docs)
            j2[std::to_string(doc.id)] = doc.to_json();
        output << j2.dump(1);
        output.close();

        output.open("../index/index1_positions_map.json");
        json j3;
        for (const auto &item : positions_map) {
            j3[item.first] = json::object();
            for (const auto &item2 : item.second) {
                j3[item.first][std::to_string(item2.first)] = json::array();
                for (const auto &pos : item2.second)
                    j3[item.first][std::to_string(item2.first)].push_back(pos);
            }
        }
        output << j3.dump(1);
        output.close();
    }

//    /* Index documents */
//    auto indexer = Indexer(docs, tokenized_docs, positions_map);
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
//    std::cout << std::endl;
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
