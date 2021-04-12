# ----- EXAMPLE: THREADS -----
add_executable(example_threads "Source/ThreadsExample.cpp")

target_link_libraries(example_threads moxie)

target_include_directories( example_threads
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Source
		MOXIE_INTERFACE_INCLUDES
)

# Allowing reference of root path
target_compile_definitions(example_threads 
	PRIVATE 
		TEXTURES_EXAMPLE_PROJ_ROOT_PATH=${CMAKE_CURRENT_SOURCE_DIR}
)
