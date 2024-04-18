#include "Preprocessor.h"

/** Map of czech special characters to their ASCII equivalents */
std::map<std::string, std::string> czech_special_characters = {
        {"Á", "a"},
        {"Č", "c"},
        {"Ď", "d"},
        {"É", "e"},
        {"Ě", "e"},
        {"Í", "i"},
        {"Ň", "n"},
        {"Ó", "o"},
        {"Ř", "r"},
        {"Š", "s"},
        {"Ť", "t"},
        {"Ú", "u"},
        {"Ů", "u"},
        {"Ý", "y"},
        {"Ž", "z"},
        {"á", "a"},
        {"č", "c"},
        {"ď", "d"},
        {"é", "e"},
        {"ě", "e"},
        {"í", "i"},
        {"ň", "n"},
        {"ó", "o"},
        {"ř", "r"},
        {"š", "s"},
        {"ť", "t"},
        {"ú", "u"},
        {"ů", "u"},
        {"ý", "y"},
        {"ž", "z"}
};
std::string czech_special_characters_lookup = "ÁČĎÉĚÍŇÓŘŠŤÚŮÝŽáčďéěíňóřšťúůýž";
std::string special_characters = "-_\"\'@#&$~•";

/** Regular expression for wildcard words */
std::regex wildcard_word_regex = std::regex(R"(\w+\*|\*\w+|\w+\*\w+)");
/** Regular expression for URL */
std::regex url_regex = std::regex(R"((https?|ftp):\/\/[^\s/$.?#].[^\s]*)");
/** Regular expression for date */
std::regex date_regex = std::regex(R"(\d{1,2}\.\s?\d{1,2}\.\s?(\d{4})?)");
/** Regular expression for time */
std::regex time_regex = std::regex(R"(\d{1,2}:\d{1,2})");

Preprocessor::Preprocessor() {
    /* Prepare instances of stemmer and lemmatizer */
    prepare_stemmer();
    prepare_lemmatizer();
    /* Load stopwords */
    load_stopwords();
}

Preprocessor::~Preprocessor() {
    /* Cleanup */
    delete this->lemmatizer;
    if (this->stemmer) {
        sb_stemmer_delete(this->stemmer);
    }
}

bool Preprocessor::prepare_stemmer() {
    /* Set language */
    char *lang = const_cast<char *>(this->language.c_str());
    this->stemmer = sb_stemmer_new(lang, nullptr);

    /* Check if stemmer was created */
    if (this->stemmer == nullptr) {
        std::cerr << "[ERROR]: Failed to create stemmer!" << std::endl;
        return false;
    }

    return true;
}

bool Preprocessor::prepare_lemmatizer() {
    /* Load language library */
    char *file_name = const_cast<char *>(this->lemma_czech_bin.c_str());
    if (lem_load_language_library(file_name) != STATUS_OK) {
        std::cerr << "[ERROR]: Failed to open language library file!" << std::endl;
        return false;
    }

    /* Create lemmatizer */
    this->lemmatizer = new RdrLemmatizer(file_name);

    return true;
}

bool Preprocessor::load_stopwords() {
    std::ifstream file(this->stopwords_file);
    std::string line;

    while (std::getline(file, line))
        this->stopwords.insert(line);

    return true;
}

std::string &Preprocessor::to_lower(std::string &text) {
    for (char & i : text)
        i = static_cast<char>(tolower(i));
    return text;
}

std::string &Preprocessor::remove_html_tags(std::string &text) {
    size_t pos = 0;
    while ((pos = text.find('<', pos)) != std::string::npos) {
        size_t end = text.find('>', pos);
        if (end == std::string::npos)
            break;
        text.replace(pos, end - pos + 1, "");
    }
    return text;
}

std::string &Preprocessor::remove_special_characters(std::string &text) {
//    for (const auto & c : czech_special_characters_lookup) {
//        auto index = text.find(c);
//        if (index != std::string::npos)
//            text.replace(index, 1, czech_special_characters[std::string(1, c)]);
//    }

    std::stringstream ss;
    /* Iterate not with character, but with string, cuz special characters are not ascii */
    for (const auto & c : text)
        if (special_characters.find(c) == std::string::npos)
            ss << c;
    text = ss.str();
    return text;
}

std::pair<std::vector<std::string>, std::map<std::string, std::vector<int>>> Preprocessor::tokenize(const std::string &text) {
    std::vector<std::string> tokens;
    std::map<std::string, std::vector<int>> token_positions;
    std::istringstream iss(text);
    std::string token;

    while (iss >> token) {
        /* Skip special tokens and save them right away */
        if (std::regex_match(token, wildcard_word_regex) || std::regex_match(token, url_regex) ||
            std::regex_match(token, date_regex) || std::regex_match(token, time_regex)) {
            tokens.push_back(token);
            token_positions[token].push_back(static_cast<int>(tokens.size() - 1));
            continue;
        }

        /* Remove punctuation, brackets ... */
        token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char x) { return std::ispunct(x); }), token.end());

        if (token.empty())
            continue;

        /* If token contains   (non-breaking space), split it into two tokens */
        if (token.find(" ") != std::string::npos) {
            while (token.find(" ") != std::string::npos)
                token = std::regex_replace(token, std::regex(" "), " ");

            std::istringstream iss(token);
            std::string new_token;
            while (iss >> new_token) {
                tokens.push_back(new_token);
                token_positions[new_token].push_back(static_cast<int>(tokens.size() - 1));
            }

            continue;
        }

        /* If token is a combination of letters and numbers, split them into more tokens e.g.: 1a2b -> 1 a 2 b */
        if (std::any_of(token.begin(), token.end(), [](char c) { return std::isdigit(c); }) &&
            std::any_of(token.begin(), token.end(), [](char c) { return std::isalpha(c); })) {

            std::string new_token;
            for (const auto & c : token) {
                if (std::isdigit(c) && !new_token.empty() && std::isalpha(new_token.back()))
                    new_token += " ";
                else if (std::isalpha(c) && !new_token.empty() && std::isdigit(new_token.back()))
                    new_token += " ";
                new_token += c;
            }

            std::istringstream iss(new_token);
            std::string new_subtoken;
            while (iss >> new_subtoken) {
                tokens.push_back(new_subtoken);
                token_positions[new_subtoken].push_back(static_cast<int>(tokens.size() - 1));
            }

            continue;
        }

        tokens.push_back(token);
        token_positions[token].push_back(static_cast<int>(tokens.size() - 1));
    }

    return {tokens, token_positions};
}

std::string Preprocessor::stem(const std::string& word) {
    const sb_symbol *stemmed = sb_stemmer_stem(this->stemmer, reinterpret_cast<const sb_symbol *>(word.c_str()), static_cast<int>(word.size()));
    return (reinterpret_cast<const char *>(stemmed));
}

std::vector<std::string> Preprocessor::stem(const std::vector<std::string>& words) {
    std::vector<std::string> stemmed_words;

    for (const auto & word : words)
        stemmed_words.emplace_back(this->stem(word));

    return stemmed_words;
}

std::string Preprocessor::lemmatize(const std::string& word) {
    char *lemmatized = this->lemmatizer->Lemmatize(word.c_str(), nullptr);
    std::string lemma_str = std::string(lemmatized);
    free(lemmatized);
    return lemma_str;
}

std::vector<std::string> Preprocessor::lemmatize(const std::vector<std::string>& words) {
    std::vector<std::string> lemmatized_words;

    for (const auto & word : words)
        lemmatized_words.emplace_back(this->lemmatize(word));

    return lemmatized_words;
}

std::vector<std::string> &Preprocessor::remove_stopwords(std::vector<std::string> &words) {
    words.erase(std::remove_if(words.begin(), words.end(), [this](const std::string &word) {
        return this->stopwords.find(word) != this->stopwords.end();
    }), words.end());
    return words;
}

std::vector<std::string> &Preprocessor::remove_duplicates(std::vector<std::string> &words) {
    std::sort(words.begin(), words.end());
    words.erase(std::unique(words.begin(), words.end()), words.end());
    return words;
}

std::pair<std::vector<std::string>, std::map<std::string, std::vector<int>>> Preprocessor::preprocess_text(std::string text, bool lemma, bool content) {
    /* Lower case */
    text = this->to_lower(text);

    /* Remove HTML tags */
    text = this->remove_html_tags(text); /* DEFAULT: Disabled because of the nature of the data */
    /* But enabled because of bonus points for semestral work */

    /* Tokenize */
    auto [tokens, token_positions] = this->tokenize(text);

    /* Stem or lemmatize */
    if (content) {
        if (lemma)
            tokens = this->lemmatize(tokens);
        else
            tokens = this->stem(tokens);
    }

    /* Remove stopwords */
//    if (content)
//        tokens = this->remove_stopwords(tokens); /* Disabled because we need stopwords for positional index */

    /* Remove duplicates */
//    if (content)
//        tokens = this->remove_duplicates(tokens); /* Disabled because we need duplicates for TF */

    return {tokens, token_positions};
}

std::pair<std::vector<std::string>, std::map<std::string, std::vector<int>>> Preprocessor::preprocess_text(const std::vector<std::string> &text, bool lemma, bool content) {
    std::string combined = std::accumulate(text.begin(), text.end(), std::string(" "));
    return preprocess_text(combined, lemma, content);
}

int Preprocessor::bool_op_precedence(const string &op) {
    if (op == operators_map[Operator::NOT])
        return 3;
    if (op == operators_map[Operator::AND])
        return 2;
    if (op == operators_map[Operator::OR])
        return 1;
    return 0;
}

std::vector<std::string> Preprocessor::parse_bool_query(const string &query) {
    // Preprocess the query to add spaces around parentheses
    std::string preprocessed_query;
    for (char c : query) {
        if (c == '(' || c == ')') {
            preprocessed_query += ' ';
            preprocessed_query += c;
            preprocessed_query += ' ';
        } else {
            preprocessed_query += c;
        }
    }

    std::vector<std::string> tokens;
    std::vector<std::string> operators;
    std::istringstream iss(preprocessed_query);
    std::string token;

    bool first = true;
    bool is_operator = false;
    bool is_and_or = false;

    while (iss >> token) {
        if (token == "(") {
            operators.push_back(token);
            is_operator = true;
            is_and_or = false;
        } else if (token == ")") {
            while (!operators.empty() && operators.back() != "(") {
                tokens.push_back(operators.back());
                operators.pop_back();
            }
            if (!operators.empty())
                operators.pop_back(); // pop the "("
            is_operator = true;
            is_and_or = false;
        } else if (token == operators_map[Operator::AND] || token == operators_map[Operator::OR]) {
            if (first || is_and_or) {
                std::cerr << "[ERROR]: Invalid query" << std::endl;
                return {};
            }
            while (!operators.empty() && bool_op_precedence(operators.back()) >= bool_op_precedence(token)) {
                tokens.push_back(operators.back());
                operators.pop_back();
            }
            operators.push_back(token);
            is_operator = true;
            is_and_or = true;
        } else if (token == operators_map[Operator::NOT]) {
            if (!operators.empty() && operators.back() == operators_map[Operator::NOT])
                operators.pop_back();
            else
                operators.push_back(token);
            is_operator = true;
            is_and_or = false;
        } else {
            if (!first && !is_operator) {
                // If the previous token was not an operator, insert an implicit OR
                while (!operators.empty() && bool_op_precedence(operators.back()) >= bool_op_precedence(operators_map[Operator::OR])) {
                    tokens.push_back(operators.back());
                    operators.pop_back();
                }
                operators.emplace_back(operators_map[Operator::OR]);
            }
            auto [preprocessed_token, _] = preprocess_text(token, true, false);
            tokens.push_back(preprocessed_token[0]);
            is_operator = false;
            is_and_or = false;
        }
        first = false;
    }

    while (!operators.empty()) {
        tokens.push_back(operators.back());
        operators.pop_back();
    }

    return tokens;
}
