cmake_minimum_required(VERSION 3.20)

project(dtl_json LANGUAGES C VERSION 0.3.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_OPTIMIZE_DEPENDENCIES ON)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/W4)
    option(ENABLE_MSVC_ANALYZE "Enable MSVC static code analysis (/analyze)" OFF)
    if(ENABLE_MSVC_ANALYZE)
        add_compile_options(/analyze)
    endif()
else()
    add_compile_options(-Wall -Wextra)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
    endif()

    option(UNIT_TEST "Unit Test Build" OFF)
    option(LEAK_CHECK "Memory Leak Detection" OFF)
    option(DTL_JSON_BUILD_EXAMPLE "Build Example Application" OFF)
endif()

set(DTL_JSON_SANITIZERS "" CACHE STRING "Comma-separated sanitizers to enable (for example: address,undefined or thread)")
set_property(CACHE DTL_JSON_SANITIZERS PROPERTY STRINGS "" "address" "address,undefined" "thread")

if(DTL_JSON_SANITIZERS)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        add_compile_options(-fsanitize=${DTL_JSON_SANITIZERS} -fno-omit-frame-pointer)
        add_link_options(-fsanitize=${DTL_JSON_SANITIZERS})
    else()
        message(FATAL_ERROR "DTL_JSON_SANITIZERS is only supported with GCC or Clang")
    endif()
endif()

if (UNIT_TEST)
    message(STATUS "UNIT_TEST=${UNIT_TEST} (DTL_JSON)")
endif()

if (LEAK_CHECK)
    message(STATUS "LEAK_CHECK=${LEAK_CHECK} (DTL_JSON)")
endif()

# Dependencies
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT TARGET adt)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../adt/CMakeLists.txt")
            add_subdirectory(../adt ${CMAKE_CURRENT_BINARY_DIR}/adt)
        else()
            message(FATAL_ERROR "adt repository not found at ../adt")
        endif()
    endif()

    if(NOT TARGET bstr)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../bstr/CMakeLists.txt")
            add_subdirectory(../bstr ${CMAKE_CURRENT_BINARY_DIR}/bstr)
        else()
            message(FATAL_ERROR "bstr repository not found at ../bstr")
        endif()
    endif()

    if(NOT TARGET cutil)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../cutil/CMakeLists.txt")
            add_subdirectory(../cutil ${CMAKE_CURRENT_BINARY_DIR}/cutil)
        else()
            message(FATAL_ERROR "cutil repository not found at ../cutil")
        endif()
    endif()

    if(NOT TARGET dtl_type)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dtl_type/CMakeLists.txt")
            add_subdirectory(../dtl_type ${CMAKE_CURRENT_BINARY_DIR}/dtl_type)
        else()
            message(FATAL_ERROR "dtl_type repository not found at ../dtl_type")
        endif()
    endif()
endif()

### Library dtl_json
set(DTL_JSON_HEADER_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/include/dtl_json.h
)

set(DTL_JSON_SOURCE_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dtl_json_reader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dtl_json_writer.c
)

add_library(dtl_json ${DTL_JSON_HEADER_LIST} ${DTL_JSON_SOURCE_LIST})

if (UNIT_TEST)
    target_compile_definitions(dtl_json PRIVATE UNIT_TEST)
endif()

if (LEAK_CHECK)
    target_compile_definitions(dtl_json PRIVATE MEM_LEAK_CHECK)
    target_link_libraries(dtl_json PRIVATE cutil)
endif()

target_link_libraries(dtl_json PUBLIC dtl_type adt PRIVATE bstr cutil)
target_include_directories(dtl_json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
###

### Executable dtl_json_unit
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(UNIT_TEST)
        set(DTL_JSON_TEST_SUITE_LIST
            test/testsuite_dtl_json_reader.c
            test/testsuite_dtl_json_writer.c
        )

        add_executable(dtl_json_unit test/test_main.c ${DTL_JSON_TEST_SUITE_LIST})
        target_link_libraries(dtl_json_unit PRIVATE adt dtl_type dtl_json cutest cutil)
        target_include_directories(dtl_json_unit PRIVATE
            "${PROJECT_BINARY_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/test"
        )
        target_compile_definitions(dtl_json_unit PRIVATE UNIT_TEST)

        if (LEAK_CHECK)
            target_compile_definitions(dtl_json_unit PRIVATE MEM_LEAK_CHECK)
            target_link_libraries(dtl_json_unit PRIVATE cutil)
        endif()

        enable_testing()
        add_test(NAME dtl_json_unit COMMAND dtl_json_unit)
        set_tests_properties(dtl_json_unit PROPERTIES
            PASS_REGULAR_EXPRESSION "OK \\([0-9]+ tests\\)"
            LABELS "unit"
        )
    endif()

    if(DTL_JSON_BUILD_EXAMPLE)
        add_executable(dtl_json_example examples/usage.c)
        target_link_libraries(dtl_json_example PRIVATE dtl_json dtl_type adt cutil)
        target_include_directories(dtl_json_example PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )
    endif()
endif()
###
