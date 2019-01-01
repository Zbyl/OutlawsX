
set(TARGET_NAME OutlawsPlugin)

set(TARGET_SOURCES
    plugin.cpp
    )

# Required Boost components: exception, endian, range
find_package(Boost 1.68 REQUIRED COMPONENTS exception)

add_library(${TARGET_NAME} SHARED ${TARGET_SOURCES})
target_link_libraries(${TARGET_NAME} PRIVATE OutlawsLib)
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../ArbitraryFormatSerializer")
target_include_directories(${TARGET_NAME} PRIVATE "${Boost_INCLUDE_DIRS}")

SetCompilerOptions(${TARGET_NAME})

install(TARGETS ${TARGET_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}")
