# ---------- THIRD PARTY LIBS ----------
cmake_minimum_required(VERSION 3.17)

# CMake offers the possibility to add External Projects. An external project can be the equivalent of a git submodule:
# it gets downloaded and eventually it can be built and installed.
# A guide about this topic can be found here https://crascit.com/2015/07/25/cmake-gtest/
# At the moment, for Moxie Engine, I decided to go with an hybrid approach: still downloading external dependencies
# by git submodules, and then use CMake to build, install and link them to our code.

set(CMAKE_COMMAND "cmake")
set(CMAKE_GENERATOR "\"Visual Studio 17 2022\"") # TODO retrieve this automatically
set(CMAKE_BUILD_FOLDER_NAME "_builds")
set(CMAKE_BUILD_CONFIG "Debug") # Note: the configuration must be in line with the targets we are linking them against (e.g. Debug for Moxie Debug)

function(build_install_lib EXTERNAL_LIB_NAME ADDITIONAL_CMD_OPTIONS)
    # Generate CMake cache
    set(CURRENT_GENERATE_COMMAND -B${CMAKE_BUILD_FOLDER_NAME} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_CONFIG} ${ADDITIONAL_CMD_OPTIONS} -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/${INSTALLED_FOLDER_NAME}/${EXTERNAL_LIB_NAME})
    set(CURRENT_WORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_LIB_NAME}")
    message("[Moxie Third Party Libs] Generating Files For ${EXTERNAL_LIB_NAME} \n${CURRENT_GENERATE_COMMAND}")
    execute_process(COMMAND "${CMAKE_COMMAND}" ${CURRENT_GENERATE_COMMAND}
        WORKING_DIRECTORY ${CURRENT_WORK_DIR}
        # Note: I cannot use COMMAND_ERROR_IS_FATAL to check validity of the command because I am using CMake 3.18 and I do not want to upgrade it
        #COMMAND_ERROR_IS_FATAL ANY
        # Note: I cannot use the following to determine if configuration was successful because it can succeed even with error messages reported
        #ERROR_VARIABLE CMD_OUTPUT_ERROR
    )

    # Build the install target
    # Note: --config specifier is sometimes essential on Windows to prevent errors!
    execute_process(COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BUILD_FOLDER_NAME} --target install --config ${CMAKE_BUILD_CONFIG}
        WORKING_DIRECTORY ${CURRENT_WORK_DIR}
    )

    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${INSTALLED_FOLDER_NAME}/${EXTERNAL_LIB_NAME})
        message(FATAL_ERROR "Install folder for ${EXTERNAL_LIB_NAME} was not created!")
    endif()

endfunction()

# ----- ENTT -----
# Build and Install ENTT only if not present in the install dir 
if(NOT EXISTS ${INSTALLED_LIBS_DIR}/entt)
    build_install_lib("entt" "")
endif()

# ----- EIGEN -----
# Build and Install eigen only if not present in the install dir
if(NOT EXISTS ${INSTALLED_LIBS_DIR}/eigen)
    build_install_lib("eigen" "")
endif()

# ----- DX12 -----
# We do not need to build and install dx12 libraries since they are searched in the hosting machine
add_subdirectory(DX12)

# ----- DXTEX -----
# Build and Install DirectXTex only if not present in the install dir
if(NOT EXISTS ${INSTALLED_LIBS_DIR}/DirectXTex)
    build_install_lib("DirectXTex" "")
endif()

# ----- MICROSOFT -----
add_subdirectory(Microsoft)
