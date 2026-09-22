cmake_minimum_required(VERSION 3.11)

project(CacheSystem LANGUAGES CXX)

add_executable(cache)

target_sources(cache
	PRIVATE
	src/main.cpp
)

target_include_directories(cache
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/policies
)
