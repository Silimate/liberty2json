cmake_minimum_required(VERSION 3.5)

set(LIBERTY2JSON_SOURCES
    liberty2json.cpp
    STALibertyParser.cpp
)

add_executable(liberty2json
    ${LIBERTY2JSON_SOURCES}
)
target_include_directories(liberty2json PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
    ${PROJECT_SOURCE_DIR}/third_party/opensta/include/sta
    
    # OpenSTA Private
    ${PROJECT_SOURCE_DIR}/third_party/opensta/liberty
    ${PROJECT_SOURCE_DIR}/third_party/opensta/
)
target_link_libraries(liberty2json PUBLIC
    Backward::Interface
    OpenSTA
    z
)
