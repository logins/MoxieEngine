# ---------- THIRD PARTY LIBS ----------
cmake_minimum_required(VERSION 3.11)

# CMake offers the possibility to add External Projects. An external project can be the equivalent of a git submodule:
# it gets downloaded and eventually it can be built and installed.
# A guide about this topic can be found here https://crascit.com/2015/07/25/cmake-gtest/
# At the moment, for Moxie Engine, I decided to go with an hybrid approach: still downloading external dependencies
# by git submodules, and then use CMake to build, install and link them to our code.

set(CMAKE_COMMAND "cmake")
set(CMAKE_GENERATOR "Ninja")
set(CMAKE_BUILD_FOLDER_NAME "_builds")
set(INSTALLED_FOLDER_NAME "installed")
set(INSTALLED_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${INSTALLED_FOLDER_NAME})

function(build_install_lib EXTERNAL_LIB_NAME)
    # Generate CMake cache
    execute_process(COMMAND "${CMAKE_COMMAND}" -B${CMAKE_BUILD_FOLDER_NAME} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/${INSTALLED_FOLDER_NAME}/${EXTERNAL_LIB_NAME}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_LIB_NAME}"
        # Note: I cannot use COMMAND_ERROR_IS_FATAL to check validity of the command because I am using CMake 3.18 and I do not want to upgrade it
        #COMMAND_ERROR_IS_FATAL ANY
        # Note: I cannot use the following to determine if configuration was successful because it can succeed even with error messages reported
        #ERROR_VARIABLE CMD_OUTPUT_ERROR
    )

    # Build and install
    execute_process(COMMAND "${CMAKE_COMMAND}" --build . --target install
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_LIB_NAME}/${CMAKE_BUILD_FOLDER_NAME}"
    )
endfunction()

# ----- EIGEN -----
# Build and Install eigen only if not present in the install dir
if(NOT EXISTS ${INSTALLED_LIBS_DIR}/eigen)
    build_install_lib("eigen")
endif()
# Once installed we can retrieve the target with find_package https://cmake.org/cmake/help/latest/command/find_package.html
find_package(eigen3 REQUIRED PATHS ${INSTALLED_FOLDER_NAME})

if(NOT TARGET Eigen3::Eigen)
	message(FATAL_ERROR "Installed Eigen library not found!")
endif()

#Note: Results of find_package call(both CONFIG and MODULE) are intended be used in the same directory or below.
# To make the target accessible from in global scope, we need to explicitly set its property IMPORTED_GLOBAL to TRUE
set_target_properties(Eigen3::Eigen PROPERTIES IMPORTED_GLOBAL TRUE)
add_library(eigen ALIAS Eigen3::Eigen)

# ----- DX12 -----
# We do not need to build and install dx12 libraries since they are searched in the hosting machine
add_subdirectory(DX12)

# ----- DXTEX -----
# Build and Install DirectXTex only if not present in the install dir
if(NOT EXISTS ${INSTALLED_LIBS_DIR}/DirectXTex)
    build_install_lib("DirectXTex")
endif()
# Finding package and library we are looking for
find_package(DirectXTex REQUIRED PATHS ${INSTALLED_FOLDER_NAME})

if(NOT TARGET Microsoft::DirectXTex)
	message(FATAL_ERROR "Installed DirectXTex library not found!")
endif()

set_target_properties(Microsoft::DirectXTex PROPERTIES IMPORTED_GLOBAL TRUE)
add_library(DirectXTex ALIAS Microsoft::DirectXTex)


