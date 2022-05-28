/*
 D3D12UtilsInternal.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef D3D12UtilsInternal_h__
#define D3D12UtilsInternal_h__

#include <MoxUtils.h>
#include "GraphicsTypes.h"
#include "DirectXTex.h"

namespace Mox {

	void ThisIsMyInternalFunction();

	bool CheckTearingSupport();

	D3D12_COMMAND_LIST_TYPE CmdListTypeToD3D12(Mox::COMMAND_LIST_TYPE InCmdListType);

	D3D12_RESOURCE_STATES ResStateTypeToD3D12(Mox::RESOURCE_STATE InResState);

	D3D12_RESOURCE_FLAGS ResFlagsToD3D12(Mox::RESOURCE_FLAGS InResFlags);

	D3D12_HEAP_TYPE HeapTypeToD3D12(Mox::RESOURCE_HEAP_TYPE InHeapType);

	DXGI_FORMAT BufferFormatToD3D12(Mox::BUFFER_FORMAT InFormat);

	Mox::BUFFER_FORMAT BufferFormatToEngine(DXGI_FORMAT InFormat);

	DirectX::TEX_DIMENSION TextureTypeToD3D12(Mox::TEXTURE_TYPE InFormat);

	Mox::TEXTURE_TYPE TextureTypeToEngine(DirectX::TEX_DIMENSION InFormat);

	D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyTypeToD3D12(Mox::PRIMITIVE_TOPOLOGY_TYPE InPrimitiveTopologyType);

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopoToD3D12(Mox::PRIMITIVE_TOPOLOGY InPrimitiveTopo);

	D3D12_FILTER SampleFilterToD3D12(Mox::SAMPLE_FILTER_TYPE InFilterType);

	D3D12_TEXTURE_ADDRESS_MODE TextureAddressModeToD3D12(Mox::TEXTURE_ADDRESS_MODE InAddressMode);
}

#endif // D3D12UtilsInternal_h__
