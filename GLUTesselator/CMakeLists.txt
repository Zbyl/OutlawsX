set(TARGET_NAME GluTesselator)

file(GLOB TARGET_SOURCES *.[ch])
#list(REMOVE_ITEM TARGET_SOURCES main.c)

add_library(${TARGET_NAME} ${TARGET_SOURCES})
SetCompilerOptionsMinimal(${TARGET_NAME})
set_source_files_properties(main.c PROPERTIES LANGUAGE "")
set_source_files_properties(priorityq-heap.c PROPERTIES LANGUAGE "")

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
