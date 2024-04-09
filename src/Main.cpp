#include "IndexHandler.h"

/**
 * Main function
 * @return Exit code
 */
int main() {
    /* Load documents */
    auto docs = IndexHandler::load_documents("../data");

    /* Preprocess documents */
    auto preprocessor = Preprocessor();
    auto tokenized_docs = IndexHandler::preprocess_documents(preprocessor, docs);

    /* Index documents */
    auto indexer = Indexer(docs, tokenized_docs);

    /* Search "Geralt z Rivie" */
    std::string query = "Geralt z Rivie";
    auto result = IndexHandler::search(indexer, preprocessor, query, 3);

    /* Update the document, so it contains "Geralt z Rivie" more and so it is way more relevant (see Score print) */
    std::vector<int> new_doc_ids = {550};
    std::vector<Document> new_docs = {{550, {"už ne Geralt z Rivie"}, {}, {}, {}, {}, {"Geralt z Rivie Geralt z Rivie Geralt z Rivie Geralt z Rivie"}}};
    IndexHandler::update_docs(indexer, preprocessor, new_doc_ids, new_docs);

    /* Search "Geralt z Rivie" again */
    result = IndexHandler::search(indexer, preprocessor, query, 3);

    /* Remove the document completely now and see how the score of others changes too, because overall IDF changes */
    IndexHandler::remove_docs(indexer, new_doc_ids);

    /* Search "Geralt z Rivie" again and see how the score of others changes (also ID 550 is gone, shocking!) */
    result = IndexHandler::search(indexer, preprocessor, query, 3);

    /* Search "Geralt z Rivie" boolean */
    query = "NOT Geralt AND (z OR NOT NOT Rivie)";
    auto result_bool = IndexHandler::search(indexer, preprocessor, query);

    return EXIT_SUCCESS;
}