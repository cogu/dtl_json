cmake_minimum_required(VERSION 3.14)


project(dtl_json LANGUAGES C VERSION 0.1.1)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(../adt/cmake/BuildTypes.cmake)
    include(../adt/cmake/LeakCheck.cmake)
    include(../adt/cmake/UnitTest.cmake)
endif()

if (LEAK_CHECK)
    message(STATUS "LEAK_CHECK=${LEAK_CHECK} (DTL_JSON)")
endif()

if (UNIT_TEST)
    message(STATUS "UNIT_TEST=${UNIT_TEST} (DTL_JSON)")
endif()

### Library bstr
set (DTL_JSON_HEADER_LIST
    inc/dtl_json.h
)

set (DTL_JSON_SOURCE_LIST
    src/dtl_json_reader.c
    src/dtl_json_writer.c
)

add_library(dtl_json ${DTL_JSON_SOURCE_LIST} ${DTL_JSON_HEADER_LIST})
if (LEAK_CHECK)
    target_compile_definitions(dtl_json PRIVATE MEM_LEAK_CHECK)
endif()
target_link_libraries(dtl_json PRIVATE adt dtl_type bstr cutil)
target_include_directories(dtl_json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
###

### Executable dtl_json_unit

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(../adt ${CMAKE_CURRENT_BINARY_DIR}/adt)
    add_subdirectory(../bstr ${CMAKE_CURRENT_BINARY_DIR}/bstr)
    add_subdirectory(../dtl_type ${CMAKE_CURRENT_BINARY_DIR}/dtl_type)
    add_subdirectory(../cutil ${CMAKE_CURRENT_BINARY_DIR}/cutil)

    if (UNIT_TEST)
        set (DTL_JSON_TEST_SUITE_LIST
            test/testsuite_dtl_json_reader.c
            test/testsuite_dtl_json_writer.c
        )

        add_executable(dtl_json_unit test/test_main.c ${DTL_JSON_TEST_SUITE_LIST})
        target_link_libraries(dtl_json_unit PRIVATE adt dtl_type dtl_json cutest cutil)
        target_include_directories(dtl_json_unit PRIVATE
                                "${PROJECT_BINARY_DIR}"
                                "${CMAKE_CURRENT_SOURCE_DIR}/inc"
                                )
        target_compile_definitions(dtl_json_unit PRIVATE UNIT_TEST)
        if (LEAK_CHECK)
            target_compile_definitions(dtl_json_unit PRIVATE MEM_LEAK_CHECK)
        endif()
    endif()
endif()
###
