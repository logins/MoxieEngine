# ----- MOXIE LIBRARY -----
cmake_minimum_required(VERSION 3.16)

# The following will find every .h and .cpp file from Source folder
file(GLOB_RECURSE moxie_SRC
    #"Source/*.h"
    "Source/*.cpp"
    # TODO including platform specific code like D3D12 should be conditional when other Apis will be implemented 
)

add_library(moxie ${moxie_SRC})

# Getting include files from external libraries
get_target_property(EIGEN_INTERFACE_INCLUDES eigen INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(moxie #Note: Only the Public folder will be seen by the targets that will use this library!!
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Source/Graphics/Public
        ${CMAKE_CURRENT_LIST_DIR}/Source/Math/Public
        ${CMAKE_CURRENT_LIST_DIR}/Source/Public
        EIGEN_INTERFACE_INCLUDES
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Source
        ${CMAKE_CURRENT_LIST_DIR}/Source/Graphics
        ${CMAKE_CURRENT_LIST_DIR}/Source/Math
        # TODO including platform specific code like D3D12 should be conditional when other Apis will be implemented 
        ${CMAKE_CURRENT_LIST_DIR}/Source/Graphics/D3D12
)

target_link_libraries(moxie
    PUBLIC
        eigen
    PRIVATE # TODO most of these should be conditional on the platform we are using
        DirectXTex d3d12.lib dxgi.lib d3dx12.h d3dcompiler.lib dxguid.lib
)

target_compile_definitions(moxie 
	PRIVATE
		GRAPHICS_SDK_D3D12=1 # This define simulates a switch between possible graphics APIs. When defined, code will assume we chose D3D12
)

target_precompile_headers(moxie
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Source/MoxPCH.h
)

