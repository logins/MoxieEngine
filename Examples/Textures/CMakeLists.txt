# ----- EXAMPLE: TEXTURES -----
add_executable(example_textures "Source/TexturesExample.cpp" "Source/TexturesExample.h")

target_link_libraries(example_textures moxie)

target_include_directories( example_textures
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Source
		MOXIE_INTERFACE_INCLUDES
)

# Allowing reference of root path
target_compile_definitions(example_textures 
	PRIVATE 
		TEXTURES_EXAMPLE_PROJ_ROOT_PATH=${CMAKE_CURRENT_SOURCE_DIR}
)
