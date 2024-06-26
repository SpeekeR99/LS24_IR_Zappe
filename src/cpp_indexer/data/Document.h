#pragma once

#include <vector>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

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
    /** Language of the document */
    std::string lang;

    /**
     * Default constructor
     */
    Document() : id(-1), title(), toc(), h1(), h2(), h3(), content(), lang() {
        /* Nothing to do here :) */
    }

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
            id(id), title(std::move(title)), toc(toc), h1(h1), h2(h2), h3(h3), content(std::move(content)), lang() {
        /* Nothing to do here :) */
    }

    /**
     * Convert the document to JSON
     * @return JSON object
     */
    [[nodiscard]] json to_json() const {
        return {
            {"id", id},
            {"title", title},
            {"toc", toc},
            {"h1", h1},
            {"h2", h2},
            {"h3", h3},
            {"content", content},
            {"lang", lang}
        };
    }
    /**
     * Convert the document from JSON
     * @param data JSON object
     */
    void from_json(const json &data) {
        id = data["id"];
        title = data["title"];
        toc = data["toc"].get<std::vector<std::string>>();
        h1 = data["h1"].get<std::vector<std::string>>();
        h2 = data["h2"].get<std::vector<std::string>>();
        h3 = data["h3"].get<std::vector<std::string>>();
        content = data["content"];
        lang = data["lang"];
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
    /** Language of the document */
    std::string lang;

    /**
     * Default constructor
     */
    TokenizedDocument() : id(-1), title(), toc(), h1(), h2(), h3(), content(), lang() {
        /* Nothing to do here :) */
    }

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
            id(id), title(title), toc(toc), h1(h1), h2(h2), h3(h3), content(content), lang() {
        /* Nothing to do here :) */
    }

    /**
     * Get positions of a word in the document
     * @param word Word
     * @return Positions
     */
    std::vector<int> get_positions(const std::string &word, bool title_b = false) const {
        std::vector<int> positions;
        auto words = content;
        if (title_b)
            words = title;
        for (auto i = 0; i < words.size(); i++)
            if (words[i] == word)
                positions.push_back(i);
        return positions;
    }

    /**
     * Convert the document to JSON
     * @return JSON object
     */
    [[nodiscard]] json to_json() const {
        return {
            {"id", id},
            {"title", title},
            {"toc", toc},
            {"h1", h1},
            {"h2", h2},
            {"h3", h3},
            {"content", content},
            {"lang", lang}
        };
    }
    /**
     * Convert the document from JSON
     * @param data JSON object
     */
    void from_json(const json &data) {
        id = data["id"];
        title = data["title"];
        toc = data["toc"].get<std::vector<std::string>>();
        h1 = data["h1"].get<std::vector<std::string>>();
        h2 = data["h2"].get<std::vector<std::string>>();
        h3 = data["h3"].get<std::vector<std::string>>();
        content = data["content"];
        lang = data["lang"];
    }
};
