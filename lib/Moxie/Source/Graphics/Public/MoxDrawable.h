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


/*
	A Drawable contains single drawing information used by render passes.
	It is meant to be used in the render thread where most of the render code lives.
*/
class Drawable
{
public:
	Drawable(const Mox::DrawableCreationInfo& InCreationInfo);

	virtual ~Drawable();

	void SetCbShaderParamValue(Mox::SpHash InHash, Mox::ConstantBuffer* InBuffer);
	void SetTexShaderParamValue(Mox::SpHash InHash, Mox::Texture* InTexture);


	// Translation, Rotation and Scale

	// Vertex and Index data
	const Mox::VertexBuffer& m_VertexBuffer;
	const Mox::IndexBuffer& m_IndexBuffer;


	// Shader parameter hash -> Buffer
	std::unordered_map<Mox::SpHash, Mox::ConstantBuffer*> m_BufferShaderParameters;
	// Shader parameter hash -> Texture
	std::unordered_map<Mox::SpHash, Mox::Texture*> m_TextureShaderParameters;

	// If to consider backfaces as the ones to render for this drawable
	bool m_RenderBackfaces = false;

	Mox::Material& m_Material;
};

}

#endif // MoxDrawable_h__
