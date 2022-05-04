/*
 GraphicsUtils.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef GraphicsUtils_h__
#define GraphicsUtils_h__

#include "GraphicsTypes.h"
#include <memory> // for std::unique_ptr

// Note: MOX_ROOT_PATH should be coming from CMake
#define Q(x) L#x
#define LQUOTE(x) Q(x)
#define MOX_SHADERS_PATH(NAME) LQUOTE(MOX_ROOT_PATH/Shaders/NAME)

// TODO move this in general Utils (not graphics specific)
// this has taken inspiration from DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) defined in winnt.h
#define DEFINE_OPERATORS_FOR_ENUM(ENUMTYPE) \
inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) {return static_cast<ENUMTYPE>((int)a | b);} \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b){return a = static_cast<ENUMTYPE>((int)a | b);} \
inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b){return static_cast<ENUMTYPE>((int)a & b);} \
inline ENUMTYPE operator &= (ENUMTYPE &a, ENUMTYPE b){return a = static_cast<ENUMTYPE>((int)a & b);}


namespace Mox { 

	class PipelineState;
	class Device;
	struct Buffer;
	class VertexBuffer;
	struct VertexBufferView;
	class IndexBuffer;
	struct IndexBufferView;

	// Note: If we had another SDK to choose from, the same functions would be defined again returning objects from the other SDK

	void EnableDebugLayer();

	Mox::Buffer& AllocateDynamicBuffer(size_t InSize);

	Mox::VertexBufferView& AllocateVertexBufferView(Mox::VertexBuffer& InVB);
	Mox::IndexBufferView& AllocateIndexBufferView(Mox::IndexBuffer& InIB, Mox::BUFFER_FORMAT InFormat);
	Mox::ConstantBufferView& AllocateConstantBufferView(Mox::Buffer& InResource);

	Mox::Shader& AllocateShader(wchar_t const* InShaderPath);

	Mox::PipelineState& AllocatePipelineState();

	std::unique_ptr<Rect> AllocateRect(int32_t InLeft, int32_t InTop, int32_t InRight, int32_t InBottom);

	std::unique_ptr<ViewPort> AllocateViewport(float InTopLeftX, float InTopLeftY, float InWidth, float InHeight);

	// Definition for the Shader Parameter Hash value type
	using SpHash = uint64_t;

	// ----- SHADER PARAMETER HASHING -----
// At the moment shader parameter hashing is implemented as FNV-1a from https://gist.github.com/hwei/1950649d523afd03285c
// with the difference of being the 64 bit version, which should only change the prime and the offset, more info here
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
// This is not the fastest to compute, but it is straight forward 
// TODO later better passing to XXH64 as it's faster: http://cyan4973.github.io/xxHash/
	static const uint64_t FNV_PRIME = 1099511628211ULL;
	static const uint64_t OFFSET_BASIS = 14695981039346656037ULL;
	template <uint32_t N> // Compile time version
	static constexpr SpHash HashSpName(const char(&str)[N], uint32_t I = N)
	{
		return I == 1 ? (OFFSET_BASIS ^ str[0]) * FNV_PRIME : (HashSpName(str, I - 1) ^ str[I - 1]) * FNV_PRIME;
	}

}
#endif // GraphicsUtils_h__
