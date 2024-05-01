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
 * Create necessary directories if they do not exist
 */
void mkdirs() {
    if (!std::filesystem::exists("../data"))
        std::filesystem::create_directory("../data");
    if (!std::filesystem::exists("../index"))
        std::filesystem::create_directory("../index");
    if (!std::filesystem::exists("../index_file_based"))
        std::filesystem::create_directory("../index_file_based");
    if (!std::filesystem::exists("../download"))
        std::filesystem::create_directory("../download");
}

/**
 * Main function
 * @return Exit code
 */
int main(int argc, char **argv) {
    /* Parse arguments, set FILE_BASED and DETECT_LANG */
    parse_args(argc, argv);

    /* Create directories if they do not exist */
    mkdirs();

    /* Run the GUI */
    GUI gui = GUI();
    gui.run();

    return EXIT_SUCCESS;
}
