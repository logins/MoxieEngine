# ----- MOXIE LIBRARY -----
cmake_minimum_required(VERSION 3.16)

# The following will find every .h and .cpp file from Source folder
file(GLOB_RECURSE moxie_SRC
    #"Source/*.h"
    "Source/*.cpp"
    # TODO including platform specific code like D3D12 should be conditional when other Apis will be implemented 
)

# Getting all the header files... this is the best method I have found on the web, first taking all the files and then filter directories
file(GLOB_RECURSE subdir_list LIST_DIRECTORIES true ${CMAKE_CURRENT_LIST_DIR}/*)

SET(moxie_public_dirlist "")
SET(moxie_private_dirlist "")

FOREACH(child ${subdir_list})
    IF(IS_DIRECTORY ${child})
        IF(${child} MATCHES "Public$")
        LIST(APPEND moxie_public_dirlist ${child})
        ELSE()
        LIST(APPEND moxie_private_dirlist ${child})
        ENDIF()
    ENDIF()
ENDFOREACH()


message( "---- CONTENT OF PUBLIC HEADERS -----\n" "${moxie_public_dirlist}" )
message( "---- CONTENT OF PRIVATE HEADERS -----\n" "${moxie_private_dirlist}" ) 

add_library(moxie ${moxie_SRC})

# ----- Dependencies -----

# Once installed we can retrieve the target with find_package https://cmake.org/cmake/help/latest/command/find_package.html
# Find EnTT
find_package(EnTT REQUIRED)
if(NOT TARGET EnTT::EnTT)
	message(FATAL_ERROR "Installed EnTT::EnTT library not found!")
endif()
add_library(entt ALIAS EnTT::EnTT)

# Find Eigen
find_package(eigen3 REQUIRED PATHS ${INSTALLED_FOLDER_NAME})
if(NOT TARGET Eigen3::Eigen)
	message(FATAL_ERROR "Installed Eigen library not found!")
endif()
add_library(eigen ALIAS Eigen3::Eigen)

# Find DirectXTex
# Finding package and library we are looking for
find_package(DirectXTex REQUIRED PATHS ${INSTALLED_FOLDER_NAME})
if(NOT TARGET Microsoft::DirectXTex)
	message(FATAL_ERROR "Installed DirectXTex library not found!")
endif()
add_library(DirectXTex ALIAS Microsoft::DirectXTex)

# Getting include files from external libraries
get_target_property(EIGEN_INTERFACE_INCLUDES eigen INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(ENTT_INTERFACE_INCLUDES entt INTERFACE_INCLUDE_DIRECTORIES)

message( "---- CONTENT OF EIGEN HEADERS ----\n" "${EIGEN_INTERFACE_INCLUDES}" )
message( "---- CONTENT OF ENTT HEADERS ----\n" "${ENTT_INTERFACE_INCLUDES}" )

target_include_directories(moxie #Note: Only the Public folder will be seen by the targets that will use this library!!
    PUBLIC
        ${moxie_public_dirlist}
        ${EIGEN_INTERFACE_INCLUDES}
        ${ENTT_INTERFACE_INCLUDES}
    PRIVATE
        ${moxie_private_dirlist}
        # TODO including platform specific code like D3D12 should be conditional when other Apis will be implemented
)


target_link_libraries(moxie
    PUBLIC
        eigen entt
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

