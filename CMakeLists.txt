cmake_minimum_required(VERSION 3.20)
project(cpp_tf_idf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -m64")

include_directories(src)
include_directories(src/cpp_indexer)
include_directories(src/cpp_indexer/data)
include_directories(src/cpp_indexer/index)
include_directories(src/cpp_indexer/gui)
include_directories(src/cpp_indexer/eval)

# - - - - - IR - - - - -

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

# - - - - - GUI  - - - - -

find_package(OpenGL REQUIRED)

add_definitions(-DGLEW_STATIC)

# Including GLFW
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW with Wayland support")
set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW with X11 support")
add_subdirectory(lib/glfw)
include_directories(${GLFW_INCLUDE_DIRS})

# Including GLEW
add_subdirectory(lib/glew)
include_directories(${GLEW_INCLUDE_DIRS})

# Including ImGui
include_directories(lib/imgui)
include_directories(lib/imgui/backends)
include_directories(lib/imgui/misc/cpp)

set(
    imgui_files
    lib/imgui/imconfig.h
    lib/imgui/imgui.cpp
    lib/imgui/imgui.h
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_internal.h
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imstb_rectpack.h
    lib/imgui/imstb_textedit.h
    lib/imgui/imstb_truetype.h
    lib/imgui/misc/cpp/imgui_stdlib.cpp
    lib/imgui/misc/cpp/imgui_stdlib.h
)

set(
    imgui_impl_files
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_glfw.h
    lib/imgui/backends/imgui_impl_opengl3.cpp
    lib/imgui/backends/imgui_impl_opengl3.h
    lib/imgui/backends/imgui_impl_opengl3_loader.h
)

add_executable(
    cpp_indexer
    src/cpp_indexer/Main.cpp
    src/cpp_indexer/Const.h
    src/cpp_indexer/index/IndexHandler.h
    src/cpp_indexer/index/IndexHandler.cpp
    src/cpp_indexer/data/Document.h
    src/cpp_indexer/data/DataLoader.h
    src/cpp_indexer/data/DataLoader.cpp
    src/cpp_indexer/data/Preprocessor.h
    src/cpp_indexer/data/Preprocessor.cpp
    src/cpp_indexer/index/TF_IDF.h
    src/cpp_indexer/index/TF_IDF.cpp
    src/cpp_indexer/index/Indexer.h
    src/cpp_indexer/index/Indexer.cpp
    src/cpp_indexer/data/FileBasedLoader.h
    src/cpp_indexer/data/FileBasedLoader.cpp
    src/PyHandler.h
    src/PyHandler.cpp
    src/cpp_indexer/gui/GUI.h
    src/cpp_indexer/gui/GUI.cpp
    ${lemma_lib_src}
    ${stem_lib_src}
    ${imgui_files}
    ${imgui_impl_files}
)

add_executable(
    cpp_indexer_eval
    src/cpp_indexer/eval/Main.cpp
    src/cpp_indexer/Const.h
    src/cpp_indexer/data/Document.h
    src/cpp_indexer/eval/DataUtils.h
    src/cpp_indexer/eval/DataUtils.cpp
    src/cpp_indexer/index/IndexHandler.h
    src/cpp_indexer/index/IndexHandler.cpp
    src/cpp_indexer/index/Indexer.h
    src/cpp_indexer/index/Indexer.cpp
    src/cpp_indexer/data/Preprocessor.h
    src/cpp_indexer/data/Preprocessor.cpp
    src/cpp_indexer/data/DataLoader.h
    src/cpp_indexer/data/DataLoader.cpp
    src/cpp_indexer/index/TF_IDF.h
    src/cpp_indexer/index/TF_IDF.cpp
    src/cpp_indexer/data/FileBasedLoader.h
    src/cpp_indexer/data/FileBasedLoader.cpp
    src/PyHandler.h
    src/PyHandler.cpp
    ${lemma_lib_src}
    ${stem_lib_src}
)

target_link_libraries(
    cpp_indexer PRIVATE
        nlohmann_json::nlohmann_json
        glfw libglew_static
        ${GLFW_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${OPENGL_LIBRARY}
)

target_link_libraries(
    cpp_indexer_eval PRIVATE
        nlohmann_json::nlohmann_json
)
