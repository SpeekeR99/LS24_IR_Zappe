#pragma once

#include <vector>
#include <string>

/**
 * Class representing Document
 * My documents are stored in JSON with the following fields:
 * title, toc, h1, h2, h3, content
 * ID has to be added by the indexer
 */
class Document {
public:
    /** ID */
    int id;
    /** Title */
    std::string title;
    /** Table of contents */
    std::vector<std::string> toc;
    /** Header 1 */
    std::vector<std::string> h1;
    /** Header 2 */
    std::vector<std::string> h2;
    /** Header 3 */
    std::vector<std::string> h3;
    /** Content */
    std::string content;

    /**
     * Constructor
     * @param id ID
     * @param title Title
     * @param toc Table of contents
     * @param h1 Header 1
     * @param h2 Header 2
     * @param h3 Header 3
     * @param content Content
     */
    Document(
            int id,
            std::string title,
            const std::vector<std::string> &toc,
            const std::vector<std::string> &h1,
            const std::vector<std::string> &h2,
            const std::vector<std::string> &h3,
            std::string content
    ) :
            id(id), title(std::move(title)), toc(toc), h1(h1), h2(h2), h3(h3), content(std::move(content)) {
        /* Nothing to do here :) */
    }
};

/**
 * Same as Document, but everything is tokenized
 * ID has to be added by the indexer
 */
class TokenizedDocument {
public:
    /** ID */
    int id;
    /** Title */
    std::vector<std::string> title;
    /** Table of contents */
    std::vector<std::string> toc;
    /** Header 1 */
    std::vector<std::string> h1;
    /** Header 2 */
    std::vector<std::string> h2;
    /** Header 3 */
    std::vector<std::string> h3;
    /** Content */
    std::vector<std::string> content;

    /**
     * Constructor
     * @param id ID
     * @param title Title
     * @param toc Table of contents
     * @param h1 Header 1
     * @param h2 Header 2
     * @param h3 Header 3
     * @param content Content
     */
    TokenizedDocument(
            int id,
            const std::vector<std::string> &title,
            const std::vector<std::string> &toc,
            const std::vector<std::string> &h1,
            const std::vector<std::string> &h2,
            const std::vector<std::string> &h3,
            const std::vector<std::string> &content
    ) :
            id(id), title(title), toc(toc), h1(h1), h2(h2), h3(h3), content(content) {
        /* Nothing to do here :) */
    }
};
