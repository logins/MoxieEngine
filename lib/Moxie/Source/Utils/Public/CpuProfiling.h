/*
 CpuProfiling.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef CpuProfiling_h__
#define CpuProfiling_h__

#include <cvmarkersobj.h>


#define ENABLE_CPU_MARKERS _DEBUG

#define CONCURRENCY_VISUALIZER (ENABLE_CPU_MARKERS && COMPILER_MSVC) // Gotten from CMake side

#if CONCURRENCY_VISUALIZER

#define DEFINE_CPU_MARKER_SERIES(InName) \
	static Concurrency::diagnostic::marker_series mks##InName(#InName);

#define CPU_MARKER_SPAN(InSeriesName, InStr, ...) \
	Concurrency::diagnostic::span mySpan(mks##InSeriesName, InStr, __VA_ARGS__);

#else

#define DEFINE_CPU_MARKER_SERIES(InName)
#define CPU_MARKER_SPAN(InSeriesName, InStr, ...)

#endif


#endif // CpuProfiling_h__