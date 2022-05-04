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

add_library(moxie_impl STATIC ${moxie_SRC})

set_property(TARGET moxie_impl PROPERTY CXX_STANDARD_REQUIRED 17)

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

add_library(eigen ALIAS Eigen3::Eigen)

# Find DirectXTex
# Finding package and library we are looking for
# Need to set this variable since DirectXTex has problems with locating the cmake package files
set(directxtex_DIR ${INSTALLED_LIBS_DIR}/DirectXTex/cmake)
find_package(directxtex REQUIRED PATHS ${INSTALLED_FOLDER_NAME})

add_library(DirectXTex ALIAS Microsoft::DirectXTex)

# Getting include files from external libraries
get_target_property(EIGEN_INTERFACE_INCLUDES eigen INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(ENTT_INTERFACE_INCLUDES entt INTERFACE_INCLUDE_DIRECTORIES)

message( "---- CONTENT OF EIGEN HEADERS ----\n" "${EIGEN_INTERFACE_INCLUDES}" )
message( "---- CONTENT OF ENTT HEADERS ----\n" "${ENTT_INTERFACE_INCLUDES}" )

target_include_directories(moxie_impl #Note: Only the Public folder will be seen by the targets that will use this library!!
    PUBLIC
        ${moxie_public_dirlist}
        ${EIGEN_INTERFACE_INCLUDES} 
        ${ENTT_INTERFACE_INCLUDES}
    PRIVATE
        ${moxie_private_dirlist}
        # TODO including platform specific code like D3D12 should be conditional when other Apis will be implemented
)


target_link_libraries(moxie_impl
    PUBLIC
        
    PRIVATE # TODO most of these should be conditional on the platform we are using
        eigen entt $<${MSVC}:ConcVis> DirectXTex d3d12.lib dxgi.lib d3dx12.h d3dcompiler.lib dxguid.lib
)

target_compile_definitions(moxie_impl
    PUBLIC
        COMPILER_MSVC=${MSVC}
	PRIVATE
		GRAPHICS_SDK_D3D12=1 # This define simulates a switch between possible graphics APIs. When defined, code will assume we chose D3D12
        MOX_ROOT_PATH=${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(moxie_impl
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/Source/MoxPCH.h
)

# Note: What we are doing here is creating moxie as an interface target library from moxie_impl adding a /WHOLEARCHIVE linker directive to it.
# In this way, every target that will use moxie as a dependency, will have every public symbol defined by moxie_impl, without the linker to strip down some bits.
# This was needed as in moxie code we have some self-registering classes, such as the Render Passes.
# Without using this method, the linker would identify this classes as useless because not included anywhere. 
# When that happens the linker will strip down the corresponding translation units and so all the static code in those units will not execute!

# Note: /WHOLEARCHIVE linker directive is exclusive to MSVC and introduced from version 15 !!
# For gcc there is some other syntax that will involve --whole-archive directive.

add_library(moxie INTERFACE)

target_link_libraries(moxie
INTERFACE
    moxie_impl
)

# Ensure the whole archive of moxie_impl is linked
target_link_options(moxie
INTERFACE
    /WHOLEARCHIVE:moxie_impl
)
