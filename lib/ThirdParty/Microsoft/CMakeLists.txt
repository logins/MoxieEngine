# ----- MICROSOFT THIRD PARTY LIBS -----

cmake_minimum_required(VERSION 3.1)


# Concurrency Visualizer is just a header, so it becomes an Interface Library (it will not create build output)
add_library(ConcVis INTERFACE)
# We just include the directory of the header file as INTERFACE (so used only by the targets that will link this library)
target_include_directories(ConcVis 
	INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/ConcurrencyVisualizer
	)
