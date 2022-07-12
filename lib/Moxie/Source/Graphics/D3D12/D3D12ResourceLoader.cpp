/*
 D3D12ResourceLoader.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "D3D12ResourceLoader.h"
#include "D3D12Device.h"
#include "DirectXTex.h"
#include "DirectXTex.inl"
#include "D3D12UtilsInternal.h"

// ----- Important Note -----
// The following code is copied from DDSTextureLoader12.cpp of DirectXTex which is under MIT license. 

namespace Mox {

	

	const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

	struct DDS_PIXELFORMAT
	{
		uint32_t    size;
		uint32_t    flags;
		uint32_t    fourCC;
		uint32_t    RGBBitCount;
		uint32_t    RBitMask;
		uint32_t    GBitMask;
		uint32_t    BBitMask;
		uint32_t    ABitMask;
	};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHAPIXELS 0x00000001  // DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8
#define DDS_PAL8A       0x00000021  // DDPF_PALETTEINDEXED8 | DDPF_ALPHAPIXELS
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV
	// DDS_BUMPLUMINANCE 0x00040000


#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME


#define DDS_FOURCC      0x00000004  // DDPF_FOURCC

	struct DDS_HEADER
	{
		uint32_t        size;
		uint32_t        flags;
		uint32_t        height;
		uint32_t        width;
		uint32_t        pitchOrLinearSize;
		uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
		uint32_t        mipMapCount;
		uint32_t        reserved1[11];
		DDS_PIXELFORMAT ddspf;
		uint32_t        caps;
		uint32_t        caps2;
		uint32_t        caps3;
		uint32_t        caps4;
		uint32_t        reserved2;
	};
	struct DDS_HEADER_DXT10
	{
		DXGI_FORMAT     dxgiFormat;
		uint32_t        resourceDimension;
		uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
		uint32_t        arraySize;
		uint32_t        miscFlags2;
	};
#ifdef WIN32
	struct handle_closer { void operator()(HANDLE h) noexcept { if (h) CloseHandle(h); } };

	using ScopedHandle = std::unique_ptr<void, handle_closer>;

	inline HANDLE safe_handle(HANDLE h) noexcept { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }
#endif

	enum DDS_LOADER_FLAGS
	{
		DDS_LOADER_DEFAULT = 0,
		DDS_LOADER_FORCE_SRGB = 0x1,
		DDS_LOADER_MIP_RESERVE = 0x8,
	};

	HRESULT LoadTextureDataFromFile(
		_In_z_ const wchar_t* fileName,
		std::unique_ptr<uint8_t[]>& ddsData,
		const DDS_HEADER** header,
		const uint8_t** bitData,
		size_t* bitSize) noexcept
	{
		if (!header || !bitData || !bitSize)
		{
			return E_POINTER;
		}

		*bitSize = 0;

#ifdef WIN32
		// open the file
		ScopedHandle hFile(safe_handle(CreateFile2(fileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			OPEN_EXISTING,
			nullptr)));

		if (!hFile)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		// Get the file size
		FILE_STANDARD_INFO fileInfo;
		if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		// File is too big for 32-bit allocation, so reject read
		if (fileInfo.EndOfFile.HighPart > 0)
		{
			return E_FAIL;
		}

		// Need at least enough data to fill the header and magic number to be a valid DDS
		if (fileInfo.EndOfFile.LowPart < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
		{
			return E_FAIL;
		}

		// create enough space for the file data
		ddsData.reset(new (std::nothrow) uint8_t[fileInfo.EndOfFile.LowPart]);
		if (!ddsData)
		{
			return E_OUTOFMEMORY;
		}

		// read the data in
		DWORD bytesRead = 0;
		if (!ReadFile(hFile.get(),
			ddsData.get(),
			fileInfo.EndOfFile.LowPart,
			&bytesRead,
			nullptr
		))
		{
			ddsData.reset();
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (bytesRead < fileInfo.EndOfFile.LowPart)
		{
			ddsData.reset();
			return E_FAIL;
		}

		size_t len = fileInfo.EndOfFile.LowPart;

#else // !WIN32
		std::ifstream inFile(std::filesystem::path(fileName), std::ios::in | std::ios::binary | std::ios::ate);
		if (!inFile)
			return E_FAIL;

		std::streampos fileLen = inFile.tellg();
		if (!inFile)
			return E_FAIL;

		// Need at least enough data to fill the header and magic number to be a valid DDS
		if (fileLen < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
			return E_FAIL;

		ddsData.reset(new (std::nothrow) uint8_t[size_t(fileLen)]);
		if (!ddsData)
			return E_OUTOFMEMORY;

		inFile.seekg(0, std::ios::beg);
		if (!inFile)
		{
			ddsData.reset();
			return E_FAIL;
		}

		inFile.read(reinterpret_cast<char*>(ddsData.get()), fileLen);
		if (!inFile)
		{
			ddsData.reset();
			return E_FAIL;
		}

		inFile.close();

		size_t len = fileLen;
#endif

		// DDS files always start with the same magic number ("DDS ")
		auto dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData.get());
		if (dwMagicNumber != DDS_MAGIC)
		{
			ddsData.reset();
			return E_FAIL;
		}

		auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData.get() + sizeof(uint32_t));

		// Verify header to validate DDS file
		if (hdr->size != sizeof(DDS_HEADER) ||
			hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
		{
			ddsData.reset();
			return E_FAIL;
		}

		// Check for DX10 extension
		bool bDXT10Header = false;
		if ((hdr->ddspf.flags & DDS_FOURCC) &&
			(MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
		{
			// Must be long enough for both headers and magic value
			if (len < (sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)))
			{
				ddsData.reset();
				return E_FAIL;
			}

			bDXT10Header = true;
		}

		// setup the pointers in the process request
		*header = hdr;
		auto offset = sizeof(uint32_t) + sizeof(DDS_HEADER)
			+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
		*bitData = ddsData.get() + offset;
		*bitSize = len - offset;

		return S_OK;
	}

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

	DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf) noexcept
	{
		if (ddpf.flags & DDS_RGB)
		{
			// Note that sRGB formats are written using the "DX10" extended header

			switch (ddpf.RGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				{
					return DXGI_FORMAT_B8G8R8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0))
				{
					return DXGI_FORMAT_B8G8R8X8_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0) aka D3DFMT_X8B8G8R8

				// Note that many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assume
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

				// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				{
					return DXGI_FORMAT_R10G10B10A2_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
				{
					return DXGI_FORMAT_R16G16_UNORM;
				}

				if (ISBITMASK(0xffffffff, 0, 0, 0))
				{
					// Only 32-bit color channel format in D3D9 was R32F
					return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
				}
				break;

			case 24:
				// No 24bpp DXGI formats aka D3DFMT_R8G8B8
				break;

			case 16:
				if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				{
					return DXGI_FORMAT_B5G5R5A1_UNORM;
				}
				if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0))
				{
					return DXGI_FORMAT_B5G6R5_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0) aka D3DFMT_X1R5G5B5

				if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
				{
					return DXGI_FORMAT_B4G4R4A4_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0) aka D3DFMT_X4R4G4B4

				// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
				break;
			}
		}
		else if (ddpf.flags & DDS_LUMINANCE)
		{
			if (8 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0xff, 0, 0, 0))
				{
					return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x0f,0,0,0xf0) aka D3DFMT_A4L4

				if (ISBITMASK(0x00ff, 0, 0, 0xff00))
				{
					return DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
				}
			}

			if (16 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0xffff, 0, 0, 0))
				{
					return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
				if (ISBITMASK(0x00ff, 0, 0, 0xff00))
				{
					return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
			}
		}
		else if (ddpf.flags & DDS_ALPHA)
		{
			if (8 == ddpf.RGBBitCount)
			{
				return DXGI_FORMAT_A8_UNORM;
			}
		}
		else if (ddpf.flags & DDS_BUMPDUDV)
		{
			if (16 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0x00ff, 0xff00, 0, 0))
				{
					return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
				}
			}

			if (32 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
				}
				if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
				{
					return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
			}

			// No DXGI format maps to DDPF_BUMPLUMINANCE aka D3DFMT_L6V5U5, D3DFMT_X8L8V8U8
		}
		else if (ddpf.flags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC1_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			// While pre-multiplied alpha isn't directly supported by the DXGI formats,
			// they are basically the same as these BC formats so they can be mapped
			if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_SNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_SNORM;
			}

			// BC6H and BC7 are written using the "DX10" extended header

			if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
			{
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
			}
			if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
			{
				return DXGI_FORMAT_G8R8_G8B8_UNORM;
			}

			if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
			{
				return DXGI_FORMAT_YUY2;
			}

			// Check for D3DFORMAT enums being set here
			switch (ddpf.fourCC)
			{
			case 36: // D3DFMT_A16B16G16R16
				return DXGI_FORMAT_R16G16B16A16_UNORM;

			case 110: // D3DFMT_Q16W16V16U16
				return DXGI_FORMAT_R16G16B16A16_SNORM;

			case 111: // D3DFMT_R16F
				return DXGI_FORMAT_R16_FLOAT;

			case 112: // D3DFMT_G16R16F
				return DXGI_FORMAT_R16G16_FLOAT;

			case 113: // D3DFMT_A16B16G16R16F
				return DXGI_FORMAT_R16G16B16A16_FLOAT;

			case 114: // D3DFMT_R32F
				return DXGI_FORMAT_R32_FLOAT;

			case 115: // D3DFMT_G32R32F
				return DXGI_FORMAT_R32G32_FLOAT;

			case 116: // D3DFMT_A32B32G32R32F
				return DXGI_FORMAT_R32G32B32A32_FLOAT;

				// No DXGI format maps to D3DFMT_CxV8U8
			}
		}

		return DXGI_FORMAT_UNKNOWN;
	}

#undef ISBITMASK

	inline void AdjustPlaneResource(
		_In_ DXGI_FORMAT fmt,
		_In_ size_t height,
		_In_ size_t slicePlane,
		_Inout_ D3D12_SUBRESOURCE_DATA& res) noexcept
	{
		switch (fmt)
		{
		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			if (!slicePlane)
			{
				// Plane 0
				res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
			}
			else
			{
				// Plane 1
				res.pData = reinterpret_cast<const uint8_t*>(res.pData) + uintptr_t(res.RowPitch) * height;
				res.SlicePitch = res.RowPitch * ((static_cast<LONG>(height) + 1) >> 1);
			}
			break;

		case DXGI_FORMAT_NV11:
			if (!slicePlane)
			{
				// Plane 0
				res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
			}
			else
			{
				// Plane 1
				res.pData = reinterpret_cast<const uint8_t*>(res.pData) + uintptr_t(res.RowPitch) * height;
				res.RowPitch = (res.RowPitch >> 1);
				res.SlicePitch = res.RowPitch * static_cast<LONG>(height);
			}
			break;
		}
	}

	//--------------------------------------------------------------------------------------
	// Get surface information for a particular format
	//--------------------------------------------------------------------------------------
	HRESULT GetSurfaceInfo(
		_In_ size_t width,
		_In_ size_t height,
		_In_ DXGI_FORMAT fmt,
		size_t* outNumBytes,
		_Out_opt_ size_t* outRowBytes,
		_Out_opt_ size_t* outNumRows) noexcept
	{
		uint64_t numBytes = 0;
		uint64_t rowBytes = 0;
		uint64_t numRows = 0;

		bool bc = false;
		bool packed = false;
		bool planar = false;
		size_t bpe = 0;
		switch (fmt)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			bc = true;
			bpe = 8;
			break;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			bc = true;
			bpe = 16;
			break;

		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_YUY2:
			packed = true;
			bpe = 4;
			break;

		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			packed = true;
			bpe = 8;
			break;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_P208:
			planar = true;
			bpe = 2;
			break;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			planar = true;
			bpe = 4;
			break;

		default:
			break;
		}

		if (bc)
		{
			uint64_t numBlocksWide = 0;
			if (width > 0)
			{
				numBlocksWide = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
			}
			uint64_t numBlocksHigh = 0;
			if (height > 0)
			{
				numBlocksHigh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
			}
			rowBytes = numBlocksWide * bpe;
			numRows = numBlocksHigh;
			numBytes = rowBytes * numBlocksHigh;
		}
		else if (packed)
		{
			rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
			numRows = uint64_t(height);
			numBytes = rowBytes * height;
		}
		else if (fmt == DXGI_FORMAT_NV11)
		{
			rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
			numRows = uint64_t(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
			numBytes = rowBytes * numRows;
		}
		else if (planar)
		{
			rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
			numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
			numRows = height + ((uint64_t(height) + 1u) >> 1);
		}
		else
		{
			size_t bpp = DirectX::BitsPerPixel(fmt);
			if (!bpp)
				return E_INVALIDARG;

			rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
			numRows = uint64_t(height);
			numBytes = rowBytes * height;
		}

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
		static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
		if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
			return HRESULT_E_ARITHMETIC_OVERFLOW;
#else
		static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
#endif

		if (outNumBytes)
		{
			*outNumBytes = static_cast<size_t>(numBytes);
		}
		if (outRowBytes)
		{
			*outRowBytes = static_cast<size_t>(rowBytes);
		}
		if (outNumRows)
		{
			*outNumRows = static_cast<size_t>(numRows);
		}

		return S_OK;
	}

	HRESULT FillInitData(_In_ size_t width,
		_In_ size_t height,
		_In_ size_t depth,
		_In_ size_t mipCount,
		_In_ size_t arraySize,
		_In_ size_t numberOfPlanes,
		_In_ DXGI_FORMAT format,
		_In_ size_t maxsize,
		_In_ size_t bitSize,
		_In_reads_bytes_(bitSize) const uint8_t* bitData,
		_Out_ size_t& twidth,
		_Out_ size_t& theight,
		_Out_ size_t& tdepth,
		_Out_ size_t& skipMip,
		std::vector<D3D12_SUBRESOURCE_DATA>& initData)
	{
		if (!bitData)
		{
			return E_POINTER;
		}

		skipMip = 0;
		twidth = 0;
		theight = 0;
		tdepth = 0;

		size_t NumBytes = 0;
		size_t RowBytes = 0;
		const uint8_t* pEndBits = bitData + bitSize;

		initData.clear();

		for (size_t p = 0; p < numberOfPlanes; ++p)
		{
			const uint8_t* pSrcBits = bitData;

			for (size_t j = 0; j < arraySize; j++)
			{
				size_t w = width;
				size_t h = height;
				size_t d = depth;
				for (size_t i = 0; i < mipCount; i++)
				{
					HRESULT hr = Mox::GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, nullptr);
					if (FAILED(hr))
						return hr;

					if (NumBytes > UINT32_MAX || RowBytes > UINT32_MAX)
						return false;

					if ((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
					{
						if (!twidth)
						{
							twidth = w;
							theight = h;
							tdepth = d;
						}

						D3D12_SUBRESOURCE_DATA res =
						{
							pSrcBits,
							static_cast<LONG_PTR>(RowBytes),
							static_cast<LONG_PTR>(NumBytes)
						};

						AdjustPlaneResource(format, h, p, res);

						initData.emplace_back(res);
					}
					else if (!j)
					{
						// Count number of skipped mipmaps (first item only)
						++skipMip;
					}

					if (pSrcBits + (NumBytes * d) > pEndBits)
					{
						return false;
					}

					pSrcBits += NumBytes * d;

					w = w >> 1;
					h = h >> 1;
					d = d >> 1;
					if (w == 0)
					{
						w = 1;
					}
					if (h == 0)
					{
						h = 1;
					}
					if (d == 0)
					{
						d = 1;
					}
				}
			}
		}

		return initData.empty() ? E_FAIL : S_OK;
	}

	inline uint32_t CountMips(uint32_t width, uint32_t height) noexcept
	{
		if (width == 0 || height == 0)
			return 0;

		uint32_t count = 1;
		while (width > 1 || height > 1)
		{
			width >>= 1;
			height >>= 1;
			count++;
		}
		return count;
	}

	bool D3D12ResourceLoader::LoadTextureData(const wchar_t* InFilePath, 
		Mox::TextureDesc& OutDesc, const void*& OutData, size_t& OutSize, std::vector<Mox::TexDataInfo>& OutSubResInfo)
	{

		std::unique_ptr<uint8_t[]> ddsData;

		const DDS_HEADER* header = nullptr;
		const uint8_t* bitData = nullptr; // Pointer to the texture content
		size_t bitSize = 0; // Size (in bytes) of the texture content

		HRESULT hr = Mox::LoadTextureDataFromFile(
			InFilePath,
			ddsData,
			&header,
			&bitData,
			&bitSize
		);
		if (FAILED(hr))
		{
			return false;
		}

		
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		static const size_t maxsize = 8000000; // 8MB max tex size
		D3D12_RESOURCE_FLAGS resFlags;
		unsigned int loadFlags = 0; // No load flags needed at the moment

		// ----- INTERPRET DDS HEADER -----

		UINT width = header->width;
		UINT height = header->height;
		UINT depth = header->depth;

		D3D12_RESOURCE_DIMENSION resDim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
		UINT arraySize = 1;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		bool isCubeMap = false;

		size_t mipCount = header->mipMapCount;
		if (0 == mipCount)
		{
			mipCount = 1;
		}


		if ((header->ddspf.flags & DDS_FOURCC) &&
			(MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
		{
			auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>(reinterpret_cast<const char*>(header) + sizeof(DDS_HEADER));

			arraySize = d3d10ext->arraySize;
			if (arraySize == 0)
			{
				return false;
			}

			switch (d3d10ext->dxgiFormat)
			{
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
			case DXGI_FORMAT_A8P8:
				return false;

			default:
				if (DirectX::BitsPerPixel(d3d10ext->dxgiFormat) == 0)
				{
					return false;
				}
			}

			format = d3d10ext->dxgiFormat;


			switch (d3d10ext->resourceDimension)
			{
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				// D3DX writes 1D textures with a fixed Height of 1
				if ((header->flags & DDS_HEIGHT) && height != 1)
				{
					return false;
				}
				height = depth = 1;
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (d3d10ext->miscFlag & 0x4 /* RESOURCE_MISC_TEXTURECUBE */)
				{
					arraySize *= 6;
					isCubeMap = true;
				}
				depth = 1;
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
				{
					return false;
				}

				if (arraySize > 1)
				{
					return false;
				}
				break;

			default:
				return false;
			}

			resDim = static_cast<D3D12_RESOURCE_DIMENSION>(d3d10ext->resourceDimension);
		}
		else
		{
			format = GetDXGIFormat(header->ddspf);

			if (format == DXGI_FORMAT_UNKNOWN)
			{
				return false;
			}

			if (header->flags & DDS_HEADER_FLAGS_VOLUME)
			{
				resDim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			}
			else
			{
				if (header->caps2 & DDS_CUBEMAP)
				{
					// We require all six faces to be defined
					if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
					{
						return false;
					}

					arraySize = 6;
					isCubeMap = true;
				}

				depth = 1;
				resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

				// Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
			}

			assert(DirectX::BitsPerPixel(format) != 0);
		}

		// Bound sizes (for security purposes we don't trust DDS file metadata larger than the Direct3D hardware requirements)
		if (mipCount > D3D12_REQ_MIP_LEVELS)
		{
			return false;
		}

		Mox::TEXTURE_TYPE textureType;

		switch (resDim)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			if ((arraySize > D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
				(width > D3D12_REQ_TEXTURE1D_U_DIMENSION))
			{
				return false;
			}
			textureType = TEXTURE_TYPE::TEX_1D;
			break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if (isCubeMap)
			{
				// This is the right bound because we set arraySize to (NumCubes*6) above
				if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
					(width > D3D12_REQ_TEXTURECUBE_DIMENSION) ||
					(height > D3D12_REQ_TEXTURECUBE_DIMENSION))
				{
					return false;
				}
				textureType = TEXTURE_TYPE::TEX_CUBE;
			}
			else if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
				(width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
				(height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION))
			{
				return false;
			}
			else
			{
				textureType = TEXTURE_TYPE::TEX_2D;
			}
			break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			if ((arraySize > 1) ||
				(width > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
				(height > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
				(depth > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
			{
				return false;
			}
			textureType = TEXTURE_TYPE::TEX_3D;
			break;

		default:
			return false;
		}

		UINT numberOfPlanes = D3D12GetFormatPlaneCount(static_cast<Mox::D3D12Device&>(Mox::GetDevice()).GetInner().Get(), format);
		if (!numberOfPlanes)
			return false;

		if ((numberOfPlanes > 1) && DirectX::IsDepthStencil(format))
		{
			// DirectX 12 uses planes for stencil, DirectX 11 does not
			return false;
		}

		

		// Create the texture
		size_t numberOfResources = (resDim == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
			? 1 : arraySize;
		numberOfResources *= mipCount;
		numberOfResources *= numberOfPlanes;

		if (numberOfResources > D3D12_REQ_SUBRESOURCES)
			return false;

		subresources.reserve(numberOfResources);

		size_t skipMip = 0;
		size_t twidth = 0;
		size_t theight = 0;
		size_t tdepth = 0;
		hr = FillInitData(width, height, depth, mipCount, arraySize,
			numberOfPlanes, format,
			maxsize, bitSize, bitData,
			twidth, theight, tdepth, skipMip, subresources);

		if (SUCCEEDED(hr))
		{
			size_t reservedMips = mipCount;
			if (loadFlags & Mox::DDS_LOADER_MIP_RESERVE)
			{
				reservedMips = std::min<size_t>(D3D12_REQ_MIP_LEVELS,
					CountMips(width, height));
			}
		}

		// Fill the out parameters

		// use the subresources variable to fill in 
		OutSubResInfo.resize(subresources.size());
		auto OutSubresIt = OutSubResInfo.begin();
		auto SrcSubresIt = subresources.begin();

		// Considering a matrix of subresources with array slices as columns
		// and mips as rows, sub-resources should be stored in mip-major order
		for (uint8_t curArraySlice = 0; curArraySlice < arraySize; ++curArraySlice)
		{
			for (uint8_t curMip = 0; curMip < mipCount; ++curMip)
			{
				OutSubresIt->m_Location = SrcSubresIt->pData;
				OutSubresIt->m_RowSize = SrcSubresIt->RowPitch;
				OutSubresIt->m_TotalSize = SrcSubresIt->SlicePitch;
				OutSubresIt->m_MipLevel = curMip;
				OutSubresIt->m_SliceIndex = curArraySlice;

				++OutSubresIt;
				++SrcSubresIt;
			}
		}

		// Fill texture description

		OutDesc.m_ArraySize = arraySize;
		OutDesc.m_Width = width;
		OutDesc.m_Height = height;
		OutDesc.m_MipLevelsNum = mipCount;
		OutDesc.m_PlanesNum = numberOfPlanes;
		OutDesc.m_TexelFormat = Mox::BufferFormatToEngine(format);
		OutDesc.m_Type = textureType;

		// Fill additional output params

		OutData = bitData;
		OutSize = bitSize;

		return false;
	}

}