cmake_minimum_required(VERSION 3.24)
project(cpp_tf_idf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ --static")

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
    cpp_tf_idf
    src/main.cpp
    src/IndexHandler.h
    src/IndexHandler.cpp
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
    ${imgui_files}
    ${imgui_impl_files}
)

target_link_libraries(
    cpp_tf_idf PRIVATE
        nlohmann_json::nlohmann_json
        glfw libglew_static
        ${GLFW_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${OPENGL_LIBRARY}
)
