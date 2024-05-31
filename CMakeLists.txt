cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})
set(CMAKE_GENERATOR_PLATFORM win32)

enable_language(C ASM ASM_MASM)

project(tityfiber VERSION ${PROJECT_VESRION} LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)
set_target_properties(GTest::GTest PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")


enable_testing()

add_subdirectory(TinyFiber)

add_executable(tinyfiber_tests Test/test.cpp)

set_target_properties(tinyfiber_tests PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
target_link_options(tinyfiber_tests PUBLIC "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")
target_link_libraries(tinyfiber_tests 
				tinyfiber_lib 
				GTest::GTest						
)
add_test(tinyfiber_tests tinyfiber_tests)