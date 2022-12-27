
# Do we need to specify all the folders to include one by one?
# This link answers the question:https://cmake.org/pipermail/cmake/2010-February/035418.html
#Yes, this is deliberate. CMake wants you to specify all the include dirs
#so that when you add one (or remove one), CMake can notice that your
#project has changed and regenerate your build files for you. It's the
#same reason CMake wants you to specify the names of all your source
#files, instead of just compiling *.cpp.


add_subdirectory(DynamicBuffer)

add_subdirectory(Textures)

add_subdirectory(Threads)

add_subdirectory(EnTT)

add_subdirectory(MoxieLogoScene)
