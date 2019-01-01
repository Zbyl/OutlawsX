
set(TARGET_NAME OutlawsHelper)

set(TARGET_SOURCES
    helper_main.cpp
    )

# Required Boost components: exception, endian, range
find_package(Boost 1.68 REQUIRED COMPONENTS exception)

add_executable(${TARGET_NAME} ${TARGET_SOURCES})
target_link_libraries(${TARGET_NAME} PRIVATE OutlawsLabLib)
target_link_libraries(${TARGET_NAME} PRIVATE OutlawsLib)
target_include_directories(${TARGET_NAME} PRIVATE "${Boost_INCLUDE_DIRS}")

SetCompilerOptions(${TARGET_NAME})

if(WIN32)
    #set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "OLGEO.LAB m")
    set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
