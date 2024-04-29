#pragma once

/** File based index */
extern bool FILE_BASED;
/** Language detection is REALLY sloooow */
extern bool DETECT_LANG;
/** Use lemmatization or stemming */
extern bool USE_LEMMA;

/** Path to the index */
const std::string INDEX_PATH = "../index/";
/** Path to the file based index */
const std::string FILE_BASED_INDEX_PATH = "../index_file_based/";
