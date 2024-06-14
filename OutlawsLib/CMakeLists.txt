
# Generate ANTLR4 grammar for LVT files.
#add_custom_command(
#    OUTPUT  generated.cpp
#    COMMAND java -jar ${CMAKE_SOURCE_DIR}/../Antlr4/antlr-4.7.1-complete.jar -Dlanguage=Cpp -no-listener -visitor -o ${CMAKE_CURRENT_BINARY_DIR}/antlr antlr/lvt.g
#    DEPENDS antlr/lvt.g
#)

# Call macro to add lexer and grammar to your build dependencies.
# NOTE: Here, we define "lvtgrammar" as our project's namespace
# We need to put all output files together, so that ANTLR will find Lexer from inside the Parser.
antlr4_generate(LvtLexer ${CMAKE_CURRENT_SOURCE_DIR}/antlr/LvtLexer.g4 LEXER FALSE FALSE lvtgrammar)
antlr4_generate(LvtParser ${CMAKE_CURRENT_SOURCE_DIR}/antlr/LvtParser.g4 PARSER FALSE TRUE lvtgrammar "${ANTLR4_TOKEN_FILES_LvtLexer}" "${ANTLR4_TOKEN_DIRECTORY_LvtLexer}")

antlr4_generate(InfLexer ${CMAKE_CURRENT_SOURCE_DIR}/antlr/InfLexer.g4 LEXER FALSE FALSE infgrammar)
antlr4_generate(InfParser ${CMAKE_CURRENT_SOURCE_DIR}/antlr/InfParser.g4 PARSER FALSE TRUE infgrammar "${ANTLR4_TOKEN_FILES_InfLexer}" "${ANTLR4_TOKEN_DIRECTORY_InfLexer}")

antlr4_generate(AtxLexer ${CMAKE_CURRENT_SOURCE_DIR}/antlr/AtxLexer.g4 LEXER FALSE FALSE atxgrammar)
antlr4_generate(AtxParser ${CMAKE_CURRENT_SOURCE_DIR}/antlr/AtxParser.g4 PARSER FALSE TRUE atxgrammar "${ANTLR4_TOKEN_FILES_AtxLexer}" "${ANTLR4_TOKEN_DIRECTORY_AtxLexer}")


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
    RuntimeLevel.h
    RuntimeLevel.cpp
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

    ${ANTLR4_SRC_FILES_LvtLexer}
    ${ANTLR4_SRC_FILES_LvtParser}

    ${ANTLR4_SRC_FILES_InfLexer}
    ${ANTLR4_SRC_FILES_InfParser}

    ${ANTLR4_SRC_FILES_AtxLexer}
    ${ANTLR4_SRC_FILES_AtxParser}

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

source_group(TREE "${CMAKE_BINARY_DIR}" FILES
    ${ANTLR4_SRC_FILES_LvtLexer}
    ${ANTLR4_SRC_FILES_LvtParser}

    ${ANTLR4_SRC_FILES_InfLexer}
    ${ANTLR4_SRC_FILES_InfParser}

    ${ANTLR4_SRC_FILES_AtxLexer}
    ${ANTLR4_SRC_FILES_AtxParser}
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

message(STATUS "Found antlr4cpp includes: ${ANTLR4_INCLUDE_DIR}")
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIR})

# Add generated files to includes.
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIR_LvtLexer})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIR_LvtParser})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIR_InfLexer})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIR_InfParser})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIR_AtxLexer})
target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4_INCLUDE_DIR_AtxParser})

add_dependencies(${TARGET_NAME} antlr4_shared)
target_link_libraries(${TARGET_NAME} PUBLIC antlr4_shared)
