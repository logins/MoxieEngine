# ----- EXAMPLE: DYNAMIC BUFFER -----
add_executable(example_dynbuf Source/DynamicBufferExample.cpp "Source/DynamicBufferExample.h")

target_link_libraries(example_dynbuf moxie)

target_include_directories( example_dynbuf
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Source
		MOXIE_INTERFACE_INCLUDES
)

# Allowing reference of root path
target_compile_definitions(example_dynbuf 
	PRIVATE 
		DYN_BUF_EXAMPLE_PROJ_ROOT_PATH=${CMAKE_CURRENT_SOURCE_DIR}
)
