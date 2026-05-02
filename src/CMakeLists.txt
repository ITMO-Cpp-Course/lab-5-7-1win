add_library(resource_core STATIC
        DocumentBuilder.cpp
        InvertedIndex.cpp
)

target_include_directories(resource_core PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(resource_core PUBLIC
        project_warnings
        project_sanitizers
)

add_library(Document INTERFACE)

target_sources(Document INTERFACE
        ${CMAKE_SOURCE_DIR}/include/Document.hpp
)

target_include_directories(Document INTERFACE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(Document INTERFACE
        project_warnings
        project_sanitizers
)

target_link_libraries(resource_core PUBLIC Document)