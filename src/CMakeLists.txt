add_library(resource_core STATIC
        IndexStore.cpp
        UpdateTransaction.cpp
        DocumentBuilder.cpp
        InvertedIndex.cpp
)

target_include_directories(resource_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_library(Document INTERFACE)

target_sources(Document INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/IndexStore.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/Result.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/UpdateTransaction.hpp
)

target_include_directories(Document INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries(resource_core PUBLIC Document)
