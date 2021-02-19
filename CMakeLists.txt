cmake_minimum_required( VERSION 3.1 )

project(MoxieEngine
	DESCRIPTION "Game Engine"
	LANGUAGES CXX
	)

if(NOT DEFINED MSVC)
	message(FATAL_ERROR "At the moment this program supports MSVC compilers only.")
endif()

add_subdirectory(lib)

#add_subdirectory(Editor)

add_subdirectory(Examples)