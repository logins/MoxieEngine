/*
 D3D12ResourceLoader.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef D3D12ResourceLoader__h_
#define D3D12ResourceLoader__h_

#include "MoxResourceLoader.h"

namespace Mox {

class D3D12ResourceLoader : public Mox::ResourceLoader
{
public:

	bool LoadTextureData(const wchar_t* InFilePath,
		Mox::TextureDesc& OutDesc, std::unique_ptr<uint8_t[]>& OutDataArray,
		const void*& OutDataPtr, size_t& OutSize,
		std::vector<Mox::TexDataInfo>& OutSubResInfo) override;

};

}

#endif // D3D12ResourceLoader__h_