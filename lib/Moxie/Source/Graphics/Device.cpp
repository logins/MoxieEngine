/*
 Device.cpp

 First DX12 Renderer - https://github.com/logins/FirstDX12Renderer

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "Device.h"

#ifdef GRAPHICS_SDK_D3D12
#include "D3D12Device.h"
#endif
#include "Public/GraphicsUtils.h"

namespace Mox { 

Mox::Device& GetDevice()
{
	static std::unique_ptr<Mox::Device> graphicsDevice = nullptr;


#ifdef GRAPHICS_SDK_D3D12
	if (!graphicsDevice) 
	{
		// Note: Debug Layer needs to be created before creating the Device
		Mox::EnableDebugLayer();

		graphicsDevice = std::make_unique<Mox::D3D12Device>();
	}
#endif

	return *graphicsDevice;
}


}
