cmake_minimum_required( VERSION 2.8 )

option (BUILD_TESTS "Build tests" OFF)

## Common includes

include_directories(include)

## Example project
## Shows some usage of evicting cache map

set(PROJECT_NAME example)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "-g -Wall -pedantic")

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
