/*
 MoxDrawable.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef MoxDrawable_h__
#define MoxDrawable_h__

#include "MoxMath.h"
#include <unordered_map>

namespace Mox {

class ConstantBuffer;
class VertexBuffer;
class IndexBuffer;
class Material;
struct ConstantBufferView;
struct VertexBufferView;
struct IndexBufferView;



class Drawable
{
public:
	Drawable(Mox::VertexBuffer& InVertexBuffer, Mox::IndexBuffer& InIndexBuffer, 
		const std::vector<std::tuple<Mox::SpHash, Mox::ConstantBuffer*>>& InBufferShaderParams,
		const std::vector<std::tuple<Mox::SpHash, Mox::Texture*>>& InTextureShaderParams);

	virtual ~Drawable();

	void SetCbShaderParamValue(Mox::SpHash InHash, Mox::ConstantBuffer* InBuffer);
	void SetTexShaderParamValue(Mox::SpHash InHash, Mox::Texture* InTexture);


	// Translation, Rotation and Scale

	// Vertex and Index data
	Mox::VertexBuffer& m_VertexBuffer;
	Mox::IndexBuffer& m_IndexBuffer;


	// Shader parameter hash -> Buffer
	std::unordered_map<Mox::SpHash, Mox::ConstantBuffer*> m_BufferShaderParameters;
	// Shader parameter hash -> Texture
	std::unordered_map<Mox::SpHash, Mox::Texture*> m_TextureShaderParameters;

	Mox::Material& m_Material;
};

}

#endif // MoxDrawable_h__
