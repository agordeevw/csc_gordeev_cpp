cmake_minimum_required( VERSION 3.0.2 )

option (BUILD_TESTS "Build tests" OFF)

## Common includes

include_directories(include)

## Example project
## Shows some usage of streams

set(PROJECT_NAME example)
project(${PROJECT_NAME})

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -pedantic")

add_executable (${PROJECT_NAME}
	src/example.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
	LINKER_LANGUAGE CXX
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (BUILD_TESTS)

	add_subdirectory(tests)

endif (BUILD_TESTS)
