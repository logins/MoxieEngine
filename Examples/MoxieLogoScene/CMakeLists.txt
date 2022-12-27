# ----- Example application: Moxie Logo Scene -----
add_executable(moxie_logo_scene "Source/MoxieLogoScene.cpp" "Source/MoxieLogoScene.h")

target_link_libraries(moxie_logo_scene moxie)

target_include_directories( moxie_logo_scene
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Source
		MOXIE_INTERFACE_INCLUDES
)

# Allowing reference of root path
target_compile_definitions(moxie_logo_scene 
	PRIVATE 
		MOXIE_LOGO_PROJ_ROOT_PATH=${CMAKE_CURRENT_SOURCE_DIR}
)
