cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(OutlawsX LANGUAGES CXX)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(ANTLR4CPP_JAR_LOCATION ${CMAKE_SOURCE_DIR}/../Antlr4/antlr-4.7.2-complete.jar)

# compiler must be 11 or 14
SET(CMAKE_CXX_STANDARD 14)
# add external build for antlrcpp
include(ExternalAntlr4Cpp)

function(SetCompilerOptions targetName)
    # Recommended warnings mostly taken from https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
    if(MSVC)
        set(optionsList
            /permissive-    # Enforces standards conformance.
            /Zc:__cplusplus # Set proper value of __cplusplus macro.

            #/Wx     # Treat warnings as errors
            /W4     # All reasonable warnings
            /w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss of data
            /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
            #/w14263 # 'function': member function does not override any base class virtual member function
            /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
            /w14287 # 'operator': unsigned/negative constant mismatch
            /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
            /w14296 # 'operator': expression is always 'boolean_value'
            /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
            /w14545 # expression before comma evaluates to a function which is missing an argument list
            /w14546 # function call before comma missing argument list
            /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
            /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
            /w14555 # expression has no effect; expected expression with side-effect
            /w14619 # pragma warning: there is no warning number 'number'
            /w14640 # Enable warning on thread un-safe static member initialization
            /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
            /w14905 # wide string literal cast to 'LPSTR'
            /w14906 # string literal cast to 'LPWSTR'
            /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied

            /wd4251 # ANTLR4 generates those. class '...' needs to have dll-interface to be used by clients of class 'antlr4::...'
            /wd4263 # ANTLR4 generates those. member function does not override any base class virtual member function
        )

        set(linkerOptionsList
            /ignore:4221 # ANTLR4 generates those. This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
        )
    else()
        set(optionsList
            #-Werror                     # Treat warnings as errors
            -Wall -Wextra               # reasonable and standard
            #-Wshadow                    # warn the user if a variable declaration shadows one from a parent context
            -Wnon-virtual-dtor          # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
            -Wold-style-cast            # warn for c-style casts
            -Wcast-align                # warn for potential performance problem casts
            -Wunused                    # warn on anything being unused
            -Woverloaded-virtual        # warn if you overload (not override) a virtual function
            #-Wpedantic                  # warn if non-standard C++ is used
            -Wconversion                # warn on type conversions that may lose data
            #-Wmisleading-indentation    # (not supported on old gcc) warn if indentation implies blocks where blocks do not exist
            #-Wduplicated-cond           # (not supported on old gcc) warn if if / else chain has duplicated conditions
            #-Wduplicated-branches       # (not supported on old gcc) warn if if / else branches have duplicated code
            #-Wnull-dereference          # (not supported on old gcc) warn if a null dereference is detected
            -Wdouble-promotion          # warn if float is implicit promoted to double
            -Wformat=2                  # warn on security issues around functions that format output (ie printf)
            #-Wno-variadic-macros		# Don't complain about non-standard use of macros.
        )
        if (USING_CLANG)
            list(APPEND optionsList
                #-Wlifetime                  # (clang only) shows object lifetime issues
                -Wno-sign-conversion        # don't warn on sign conversions (this is very noisy)
            )
        else()
            list(APPEND optionsList
                -Wlogical-op                # (gcc only) warn about logical operations being used where bitwise were probably wanted
                -Wuseless-cast              # (gcc only) warn if you perform a cast to the same type
            )
        endif()
        set(linkerOptionsList
        )
    endif()
    target_compile_options(${targetName} PRIVATE ${optionsList})
    target_link_options(${targetName} PRIVATE ${linkerOptionsList})

    # As this is a public compile feature anything that links to target will also build with -std=c++14
    target_compile_features(${targetName} PUBLIC cxx_std_14)

    if(MSVC)
        # Get rid of annoying compiler warnings.
        target_compile_definitions(${targetName} PUBLIC _CRT_SECURE_NO_WARNINGS)
        target_compile_definitions(${targetName} PUBLIC _SCL_SECURE_NO_WARNINGS)

        target_compile_definitions(${targetName} PUBLIC NOMINMAX)
        target_compile_definitions(${targetName} PUBLIC WIN32_LEAN_AND_MEAN)
        target_compile_definitions(${targetName} PUBLIC VC_EXTRALEAN)
    endif()
endfunction()

add_subdirectory(OutlawsLib)
add_subdirectory(OutlawsLabLib)
add_subdirectory(OutlawsPlugin)
add_subdirectory(LabFuse)
add_subdirectory(OutlawsHelper)

if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT lab-fuse)
endif()
