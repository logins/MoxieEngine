/*
 MoxResourceLoader.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxResourceLoader.h"
#ifdef GRAPHICS_SDK_D3D12
#include "D3D12/D3D12ResourceLoader.h"
#endif

namespace Mox {

Mox::ResourceLoader& ResourceLoader::Get()
{
	static ResourceLoader& instance =
#ifdef GRAPHICS_SDK_D3D12
	 *new Mox::D3D12ResourceLoader();
#endif

	return instance;
}

}