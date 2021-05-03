/*
 CpuProfiling.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef CpuProfiling_h__
#define CpuProfiling_h__

#include <cvmarkersobj.h>

namespace Mox {

#define ENABLE_CPU_MARKERS _DEBUG

#define CONCURRENCY_VISUALIZER (ENABLE_CPU_MARKERS && COMPILER_MSVC) // Gotten from CMake side

#if CONCURRENCY_VISUALIZER

Concurrency::diagnostic::marker_series ppa_series("Moxie");

#define CPU_MARKER_SPAN(InStr, ...) \
	Concurrency::diagnostic::span mySpan(Mox::ppa_series, InStr, __VA_ARGS__); 

#else

#define CPU_MARK_SPAN(InSpanName)

#endif

}

#endif // CpuProfiling_h__