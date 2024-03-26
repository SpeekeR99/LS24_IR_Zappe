cmake_minimum_required(VERSION 3.24)
project(cpp_tf_idf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ --static")

# Add lemma
include_directories(lib/lemma/src)

set(
    lemma_lib_src
    lib/lemma/src/sl_lemmatizer.h
    lib/lemma/src/sl_lemmatizer.cpp
    lib/lemma/src/RdrLemmatizer.h
    lib/lemma/src/RdrLemmatizer.cpp
)

# Add stem
include_directories(lib/stem/libstemmer)
include_directories(lib/stem/runtime)
include_directories(lib/stem/src_c)

set(
    stem_lib_src
    lib/stem/libstemmer/modules.h
    lib/stem/libstemmer/libstemmer.h
    lib/stem/libstemmer/libstemmer.c
    lib/stem/runtime/header.h
    lib/stem/runtime/api.h
    lib/stem/runtime/api.c
    lib/stem/runtime/utilities.c
    lib/stem/src_c/stem_UTF_8_czech.h
    lib/stem/src_c/stem_UTF_8_german.h
    lib/stem/src_c/stem_UTF_8_english.h
    lib/stem/src_c/stem_UTF_8_czech.c
    lib/stem/src_c/stem_UTF_8_german.c
    lib/stem/src_c/stem_UTF_8_english.c
)

# Add json
add_subdirectory(lib/json)
include_directories(lib/json/include)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_executable(
    cpp_tf_idf
    src/main.cpp
    src/Document.h
    src/DataLoader.h
    src/DataLoader.cpp
    src/Preprocessor.h
    src/Preprocessor.cpp
    src/TF_IDF.h
    src/TF_IDF.cpp
    src/Indexer.h
    src/Indexer.cpp
    ${lemma_lib_src}
    ${stem_lib_src}
)

add_executable(
    excel_check
    src/excel_check.cpp
)

target_link_libraries(cpp_tf_idf PRIVATE nlohmann_json::nlohmann_json)
