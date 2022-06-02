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
		const std::vector<std::tuple<Mox::SpHash, Mox::ConstantBuffer*>>& InShaderParams);

	virtual ~Drawable();

	void SetShaderParamValue(Mox::SpHash InHash, Mox::ConstantBuffer* InBuffer);


	// Translation, Rotation and Scale
	Mox::Matrix4f m_ModelMatrix;

	// Vertex and Index data
	Mox::VertexBuffer& m_VertexBuffer;
	Mox::IndexBuffer& m_IndexBuffer;


	// Shader parameter hash -> Buffer
	std::unordered_map<Mox::SpHash, Mox::ConstantBuffer*> m_ShaderParameters; // TODO later replace with Mox::ResourceView*


	Mox::Material& m_Material;
};

}

#endif // MoxDrawable_h__
