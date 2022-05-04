# ----- EXAMPLE: ENTT -----
add_executable(example_entt "Source/EnTTExample.cpp")

target_link_libraries(example_entt moxie)

target_include_directories( example_entt
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/Source
		MOXIE_INTERFACE_INCLUDES
)
