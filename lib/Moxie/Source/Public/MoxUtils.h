/*
 MoxUtils.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef MoxUtils_h__
#define MoxUtils_h__

#ifdef _DEBUG
#define DEBUG_TEST 1
#include<iostream>
#else
#define DEBUG_TEST 0
#endif

namespace Mox {

	namespace Constants {
		static constexpr size_t g_MaxConcurrentFramesNum = 2;

	}

	// In a bigger application this would go in an Input class
	enum class KEYBOARD_KEY : uint32_t
	{
		KEY_V,
		KEY_ESC
	};



#define Q(x) L#x
#define LQUOTE(x) Q(x)

#define StopForFail(X) do {if(DEBUG_TEST){ std::cout << X << std::endl; __debugbreak();}} while (0); // This last will generate a breakpoint

#define DebugPrint(X) do {if(DEBUG_TEST) std::cout << X << std::endl;} while (0)

#define Check(X) if(DEBUG_TEST && !(X)) __debugbreak();

#define PrintD3dErrorBlob(X) std::cout << "Error Message: " << std::string((char*)(X->GetBufferPointer()),X->GetBufferSize()) << std::endl;



}

#endif // MoxUtils_h__