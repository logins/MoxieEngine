# ----- EXAMPLE: THREADS -----
add_executable(example_threads "Source/ThreadsExample.cpp")

target_link_libraries(example_threads moxie)

target_include_directories( example_threads
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Source
		MOXIE_INTERFACE_INCLUDES
)
