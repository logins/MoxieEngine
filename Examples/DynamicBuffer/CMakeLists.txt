# ----- EXAMPLE: DYNAMIC BUFFER -----
add_executable(example_dynbuf Source/DynamicBufferExample.cpp "Source/DynamicBufferExample.h")

if(NOT TARGET moxie)
	message(FATAL_ERROR "Moxie not found!")
endif()

target_link_libraries(example_dynbuf PRIVATE moxie)

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
