
# Generate ANTLR4 grammar for LVT files.
#add_custom_command(
#    OUTPUT  generated.cpp
#    COMMAND java -jar ${CMAKE_SOURCE_DIR}/../Antlr4/antlr-4.7.1-complete.jar -Dlanguage=Cpp -no-listener -visitor -o ${CMAKE_CURRENT_BINARY_DIR}/antlr antlr/lvt.g
#    DEPENDS antlr/lvt.g
#)

# Call macro to add lexer and grammar to your build dependencies.
# NOTE: Here, we define "lvtgrammar" as our project's namespace
antlr4cpp_process_grammar(XXX lvtgrammar "${CMAKE_CURRENT_SOURCE_DIR}/antlr/lvt.g")

set(TARGET_NAME OutlawsLib)

set(TARGET_SOURCES
    antlr/lvt.g
    Lvt.h
    Lvt.cpp
    ${antlr4cpp_src_files_lvtgrammar}
    )

# Required Boost components: exception, endian, range
find_package(Boost 1.68 REQUIRED COMPONENTS exception)

add_library(${TARGET_NAME} ${TARGET_SOURCES})
SetCompilerOptions(${TARGET_NAME})

target_link_libraries(${TARGET_NAME} PRIVATE OutlawsLabLib)

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../ArbitraryFormatSerializer")
target_include_directories(${TARGET_NAME} PRIVATE "${Boost_INCLUDE_DIRS}")

target_include_directories(${TARGET_NAME} PRIVATE ${ANTLR4CPP_INCLUDE_DIRS})
target_link_directories(${TARGET_NAME} PUBLIC ${ANTLR4CPP_LIBS} )
message(STATUS "Found antlr4cpp libs: ${ANTLR4CPP_LIBS} and includes: ${ANTLR4CPP_INCLUDE_DIRS} ")

# include generated files in project environment
target_include_directories(${TARGET_NAME} PRIVATE ${antlr4cpp_include_dirs_lvtgrammar})

add_dependencies(${TARGET_NAME} antlr4cpp antlr4cpp_generation_lvtgrammar)
target_link_libraries(${TARGET_NAME} PUBLIC antlr4-runtime)
