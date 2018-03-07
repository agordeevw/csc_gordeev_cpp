cmake_minimum_required( VERSION 2.8 )

project( example )

option (BUILD_TESTS "Build tests OFF")

add_executable ( example
	include/EvictingCacheMap.cpp )

target_include_directories( example PRIVATE
	include)

set_target_properties( example PROPERTIES
	LINKER_LANGUAGE CXX
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO)

if (BUILD_TESTS)
	# download latest gtest version
	# build tests
	# ???
	# profit!

endif (BUILD_TESTS)
