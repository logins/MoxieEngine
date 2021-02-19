# ----- MOXIE EDITOR -----
add_executable(moxie_editor Source/Editor.cpp "Source/Editor.h")

target_link_libraries(moxie_editor moxie)

target_include_directories( moxie_editor
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Source
		MOXIE_INTERFACE_INCLUDES
)

# Allowing reference of Moxie Editor root path
target_compile_definitions(moxie_editor 
	PRIVATE 
		EDITOR_PROJ_ROOT_PATH=${CMAKE_CURRENT_SOURCE_DIR}
)
