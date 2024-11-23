cmake_minimum_required(VERSION 3.31)
project(sus VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD_FLAGS "${CMAKE_C_STANDARD_FLAGS} -Wall -Wextra -pedantic")

# GoogleTest requires
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "lib/*.h" "lib/*.c" )
include_directories(${CMAKE_SOURCE_DIR}/lib/)
add_library(CommonLib
    ${LIB_SOURCES}
)
add_library(TestsLib
    ${LIB_SOURCES}
)

function(add_executable_custom _target _sources)
    add_executable(${_target} ${_sources})

    target_link_libraries(${_target} CommonLib)

    add_custom_target(run_${_target}
        COMMAND ./${_target}
        DEPENDS ${_target}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

enable_testing()

include(GoogleTest)

# Enable tests
file(GLOB_RECURSE TESTS_SOURCES CONFIGURE_DEPENDS "test/*.cpp" )
add_executable(tests
    ${TESTS_SOURCES}
)
target_link_libraries(tests gtest gtest_main CommonLib TestsLib)
add_custom_target(run_tests
    COMMAND ./tests
    DEPENDS tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
gtest_discover_tests(tests)

# Executables
add_executable_custom(sus "src/main.c")
