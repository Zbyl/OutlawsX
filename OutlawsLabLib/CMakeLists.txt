set(TARGET_NAME OutlawsLabLib)

set(TARGET_SOURCES
    FileSerializer.h
    Lab.h
    Lab.cpp
    )

# Required Boost components: exception, endian, range
find_package(Boost 1.68 REQUIRED COMPONENTS exception)

add_library(${TARGET_NAME} ${TARGET_SOURCES})
SetCompilerOptions(${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../ArbitraryFormatSerializer")
target_include_directories(${TARGET_NAME} PRIVATE "${Boost_INCLUDE_DIRS}")
