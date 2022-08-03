/*
 MoxResourceLoader.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef MoxResourceLoader__h_
#define MoxResourceLoader__h_

namespace Mox {

/* Helper class to load graphics resource data from files. */
class ResourceLoader
{
public:
	static ResourceLoader& Get();

	virtual bool LoadTextureData(const wchar_t* InFilePath, 
		Mox::TextureDesc& OutDesc, std::unique_ptr<uint8_t[]>& OutDataArray,
		const void*& OutDataPtr, size_t& OutSize,
		std::vector<Mox::TexDataInfo>& OutSubResInfo) = 0;

protected:
	ResourceLoader() = default;
};

}

#endif // MoxResourceLoader__h_