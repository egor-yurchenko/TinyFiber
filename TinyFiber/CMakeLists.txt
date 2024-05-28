cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

set(TINY_SOURCES
			fiber.cpp
			exception.cpp
			Coroutine.cpp
			context.cpp
			mmap_allocation.cpp
			panic.cpp
			scheduler.cpp
			stack.cpp
			switch.asm
)

set(TINY_HEADERS 
			fiber.h
			exception.h
			Coroutine.h
			context.hpp
			mmap_allocation.h
			panic.hpp
			scheduler.hpp
			stack.hpp
			string_builder.hpp
			memspan.hpp
			assert.hpp
)

add_executable(tinyfiber 
				main.cpp
				${TINY_SOURCES}
				${TINY_HEADERS}
)

target_link_options(tinyfiber PUBLIC "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")
set_target_properties(tinyfiber PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

add_library(tinyfiber_lib
				${TINY_SOURCES}
				${TINY_HEADERS}			
)

target_include_directories(tinyfiber_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_options(tinyfiber_lib PUBLIC "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")
set_target_properties(tinyfiber_lib PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")