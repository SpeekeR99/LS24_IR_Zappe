# Complex Information Retrieval System Implementation

This project implements a complex information retrieval (IR) system with various functionalities, including indexing, searching, and a graphical user interface (GUI).

## Implementation

The application is primarily implemented in C++, with the crawler and language detection implemented in Python.

### Data Loading and Preprocessing

*   **Data Source:** The system is designed to work with data crawled from `https://zaklinac.fandom.com/wiki/ZaklnaÄ‰_Wiki`.  The expected format is JSON with keys: `title`, `toc`, `h1`, `h2`, `h3`, and `content`, where each key contains an array of strings. Sample data is available at: `https://drive.google.com/drive/folders/1jFOxtOwzfwmeruiqLLb7ncQo08mDbBEM?usp=sharing`.

*   **Preprocessing Steps:**
    *   Lowercase conversion.
    *   HTML tag handling (though should not be needed due to crawler processing).
    *   Custom tokenization, including positional indexing. The tokenizer handles special tokens like dates, hyperlinks, and times. It also removes punctuation and whitespace. Numbers are handled by splitting tokens into word and number parts.
    *   Stop word removal is disabled to preserve positional information.
    *   Lemmatization (or stemming) is performed.
    *   Duplicate token removal is possible but not used.

### Indexing

*   A comprehensive `Indexer` class is implemented, containing tokenized data, a cache of original documents, keywords for GUI suggestions, the index itself (separate indices for titles and content), precomputed document norms, and a positional map.
*   The index is implemented as an inverted list: a map of words to their IDF score and a list of document IDs where the word appears, along with TF-IDF scores.
*   Indexing involves preprocessing documents, storing them in a cache (original and preprocessed forms), calculating TF-IDF scores, precomputing document norms, and invoking Python code for language detection.

### Searching

*   **Vector Space Model:**
    *   The query is preprocessed.
    *   TF-IDF scores are calculated for the query.
    *   Cosine similarity is used to compute the similarity between documents and the query (in titles and content separately).
    *   The title weight is adjusted (1.5 * title weight + content weight).
    *   Results are initially ranked by score and truncated.
    *   Optional proximity-based searching is performed using the positional index.
*   **Boolean Model:**
    *   A custom parser is implemented to process Boolean queries.
    *   The parser converts the query to postfix notation.
    *   The search is implemented using a stack-based approach. AND, OR, and NOT operators perform set intersections, unions, and differences, respectively.

### Graphical User Interface

*   The GUI is built using `imgui` (`https://github.com/ocornut/imgui`), a minimalist graphics library.
*   The main window offers tabs for "Indexing" and "Search".

    *   **Indexing Tab:** Allows creating, deleting, and loading indices. Documents can be retrieved, edited, created, and deleted. Web content from the specified wiki can be indexed.
    *   **Search Tab:** Allows selecting the index and configuring search parameters (model, fields, number of results). The interface includes keyword suggestions and displays snippets from relevant documents with highlighted search terms.

### Implemented Advanced Features

*   **File-Based Index:** The `--file-based` flag enables loading indexes from the `/index_file_based/` directory, saving RAM at the cost of some performance.

*   **Incremental Indexing:** CRUD (Create, Read, Update, Delete) operations are supported via the GUI.

*   **HTML Tag Handling:** Implemented during preprocessing.

*   **Language Detection:** Python code is used for language detection of both queries and indexed documents, integrated into the C++ application using a custom `exec()` function.

*   **Proximity Search:** Enabled via the positional index.

*   **Phrase Search:** A special case of proximity search (distance = 1).

*   **Web Content Indexing:** Indexes content from `https://zaklinac.fandom.com/wiki/ZaklnaÄ‰_Wiki`.

*   **GUI:** Implemented using `imgui`.

*   **Keyword Suggestions:** Uses a set of keywords extracted during indexing. The GUI suggests keywords as the user types.

*   **Multiple Fields:** Supports searching in titles, content, or both, with configurable weight for titles.

*   **Highlighted Search Results:** The GUI highlights search terms in the displayed snippets using the positional index.

*   **Custom Query Parsing:** Custom parsers for both general and Boolean queries are implemented.

## Evaluation and Results

Evaluation was performed using provided data (TREC Eval). Lemmatization was found to be superior to stemming. The best Mean Average Precision (MAP) achieved was 9.2%, which is considered low.

| Query Part          | MAP    |
| ------------------- | ------ |
| Lemmatization       |        |
| Title + Description | 0.0801 |
| Title               | 0.0924 |
| Description         | 0.0707 |
| Stemming            |        |
| Title + Description | 0.0160 |
| Title               | 0.0221 |
| Description         | 0.0143 |

## User Guide

The application is primarily written in C++. Python 3+ is required for web content indexing and language detection (tested with Python 3.7+).

### Build

CMake (version 3.20+) is required.

```bash
cd <project_root>/build
cmake ..
cmake --build . --config Release # Windows
make # Linux
```

Install Python dependencies:

```bash
pip install -r requirements.txt
```

### Execution

Executable files are located in `/bin/`. If compiling from source, they are in `/build/Release` (Windows) or `/build/` (Linux).

Important: Execute the application from the `/bin/` directory or relative paths may fail.

Command-line arguments:

*   `--file-based`: Use file-based index.
*   `--no-lang-detect`: Disable language detection.
*   `--lemma`: Use lemmatization.
*   `--stem`: Use stemming.
