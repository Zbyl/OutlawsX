
# Generate ANTLR4 grammar for LVT files.
#add_custom_command(
#    OUTPUT  generated.cpp
#    COMMAND java -jar ${CMAKE_SOURCE_DIR}/../Antlr4/antlr-4.7.1-complete.jar -Dlanguage=Cpp -no-listener -visitor -o ${CMAKE_CURRENT_BINARY_DIR}/antlr antlr/lvt.g
#    DEPENDS antlr/lvt.g
#)

# Call macro to add lexer and grammar to your build dependencies.
# NOTE: Here, we define "lvtgrammar" as our project's namespace
# We need to put all output files together, so that ANTLR will find Lexer from inside the Parser.
antlr_target(LvtLexer antlr/LvtLexer.g4 LEXER PACKAGE lvtgrammar OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated)
antlr_target(LvtParser antlr/LvtParser.g4 PARSER PACKAGE lvtgrammar DEPENDS_ANTLR LvtLexer VISITOR OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated)

antlr_target(InfLexer antlr/InfLexer.g4 LEXER PACKAGE infgrammar OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated)
antlr_target(InfParser antlr/InfParser.g4 PARSER PACKAGE infgrammar DEPENDS_ANTLR InfLexer VISITOR OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated)

antlr_target(AtxLexer antlr/AtxLexer.g4 LEXER PACKAGE atxgrammar OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated)
antlr_target(AtxParser antlr/AtxParser.g4 PARSER PACKAGE atxgrammar DEPENDS_ANTLR AtxLexer VISITOR OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated)


set(TARGET_NAME OutlawsLib)

set(TARGET_SOURCES
    antlr/LvtLexer.g4
    antlr/LvtParser.g4
    antlr/AtxLexer.g4
    antlr/AtxParser.g4
    antlr/InfLexer.g4
    antlr/InfParser.g4

    Lvt.h
    Lvt.cpp
    LevelRuntime.h
    LevelRuntime.cpp
    TexInfos.h
    TexInfos.cpp
    Inf.h
    Inf.cpp
    Atx.h
    Atx.cpp
    geometry.h
    geometry.cpp

    # Hack. Remove it from here.
    file_utilities.cpp

    ${ANTLR_LvtLexer_CXX_OUTPUTS}
    ${ANTLR_LvtParser_CXX_OUTPUTS}

    ${ANTLR_InfLexer_CXX_OUTPUTS}
    ${ANTLR_InfParser_CXX_OUTPUTS}

    ${ANTLR_AtxLexer_CXX_OUTPUTS}
    ${ANTLR_AtxParser_CXX_OUTPUTS}

    NatvisFile.natvis
    )

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    antlr/LvtLexer.g4
    antlr/LvtParser.g4
    antlr/AtxLexer.g4
    antlr/AtxParser.g4
    antlr/InfLexer.g4
    antlr/InfParser.g4
)

source_group(TREE "${CMAKE_CURRENT_BINARY_DIR}" FILES
    ${ANTLR_LvtLexer_CXX_OUTPUTS}
    ${ANTLR_LvtParser_CXX_OUTPUTS}

    ${ANTLR_InfLexer_CXX_OUTPUTS}
    ${ANTLR_InfParser_CXX_OUTPUTS}

    ${ANTLR_AtxLexer_CXX_OUTPUTS}
    ${ANTLR_AtxParser_CXX_OUTPUTS}
)

# Required Boost components: exception, endian, range
find_package(Boost 1.68 REQUIRED COMPONENTS exception)

add_library(${TARGET_NAME} ${TARGET_SOURCES})
SetCompilerOptions(${TARGET_NAME})

target_link_libraries(${TARGET_NAME} PRIVATE OutlawsLabLib)
target_link_libraries(${TARGET_NAME} PRIVATE GluTesselator)

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../ArbitraryFormatSerializer")
target_include_directories(${TARGET_NAME} PRIVATE "${Boost_INCLUDE_DIRS}")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/third_party/nlohmann-json-3.4.0")

target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIRS})
message(STATUS "Found antlr4cpp includes: ${ANTLR4_INCLUDE_DIRS} ")

# include generated files in project environment
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR_LvtLexer_OUTPUT_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR_LvtParser_OUTPUT_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR_InfLexer_OUTPUT_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR_InfParser_OUTPUT_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR_AtxLexer_OUTPUT_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR_AtxParser_OUTPUT_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC antlr4_static)
