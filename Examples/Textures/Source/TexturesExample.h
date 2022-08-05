/*
 TexturesExample.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef TexturesExample_h__
#define TexturesExample_h__

#include "Application.h"
#include "GraphicsTypes.h"

namespace Mox {	class PipelineState; }

class TexturesExampleApplication : public Mox::Application
{
public:
	TexturesExampleApplication() = default;

	virtual void OnInitializeContent() override;
private:

	// Callbacks for main window mouse and keyboard events
	void OnMouseWheel(float InDeltaRot);
	void OnMouseMove(int32_t InX, int32_t InY);
	void OnLeftMouseDrag(int32_t InDeltaX, int32_t InDeltaY);
	void OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY);
	void OnTypingKeyPressed(Mox::KEYBOARD_KEY InPressedKey);
	void OnControlKeyPressed(Mox::KEYBOARD_KEY InPressedKey);

	// Note: most of the following member variables should not belong to the application
	// but instead to a draw command object for the current entity being drawn.
	// In that case we would create the draw command object with references instead of raw pointers.
	// Vertex buffer for the cube
	Mox::VertexBuffer* m_CubemapVertexBuffer;
	// Index buffer for the cube
	Mox::IndexBuffer* m_CubemapIndexBuffer;

	// Texture for the cubemap
	std::unique_ptr<Mox::Texture> m_CubemapTexture;

	Mox::Entity* m_CubeEntity;

	// Vertex data for colored cube
	struct CubemapVertex
	{
		Mox::Vector3f Position;
		Mox::Vector3f CubemapCoords;
	};

	const Mox::INPUT_LAYOUT_DESC m_CubemapVertexLayoutDesc = {
		{
			{"POSITION",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
			{"CUBETEXCOORD",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
		}
	};

	const CubemapVertex m_CubemapVertexData[8] = {
		{ Mox::Vector3f(-1.f, -1.f, -1.f),Mox::Vector3f(-1.f, -1.f, -1.f) },// 0
		{ Mox::Vector3f(-1.f, 1.f, -1.f),Mox::Vector3f(-1.f, 1.f, -1.f)  },	// 1
		{ Mox::Vector3f(1.f, 1.f, -1.f), Mox::Vector3f(1.f, 1.f, -1.f)   },	// 2
		{ Mox::Vector3f(1.f, -1.f, -1.f),Mox::Vector3f(1.f, -1.f, -1.f)  },	// 3
		{ Mox::Vector3f(-1.f, -1.f, 1.f),Mox::Vector3f(-1.f, -1.f, 1.f)  },	// 4
		{ Mox::Vector3f(-1.f, 1.f, 1.f), Mox::Vector3f(-1.f, 1.f, 1.f)  },	// 5
		{ Mox::Vector3f(1.f, 1.f, 1.f),  Mox::Vector3f(1.f, 1.f, 1.f)   },	// 6
		{ Mox::Vector3f(1.f, -1.f, 1.f), Mox::Vector3f(1.f, -1.f, 1.f)  }	// 7
	};

	const unsigned short m_CubemapIndexData[36] = {
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

	// ----- Quad-related data -----
	Mox::VertexBuffer* m_QuadVertexBuffer;
	Mox::IndexBuffer* m_QuadIndexBuffer;
	std::unique_ptr<Mox::Texture> m_QuadTexture;

	Mox::Entity* m_QuadEntity;

	struct QuadVertex {
		Mox::Vector3f Position;
		Mox::Vector2f TexCoords;
	};
	const Mox::INPUT_LAYOUT_DESC m_QuadVertexLayoutDesc = {
		{
			{"POSITION",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
			{"TEXCOORD",Mox::BUFFER_FORMAT::R32G32_FLOAT},
		}
	};
	const QuadVertex m_QuadVertexData[4] = {
		{ Mox::Vector3f(-1.f, -1.f, 0.f),	Mox::Vector2f(0.f, 1.f)  },	// 0
		{ Mox::Vector3f(-1.f, 1.f, 0.f),	Mox::Vector2f(0.f, 0.f)  },	// 1
		{ Mox::Vector3f(1.f, 1.f, 0.f),		Mox::Vector2f(1.f, 0.f)  }, // 2
		{ Mox::Vector3f(1.f, -1.f, 0.f),	Mox::Vector2f(1.f, 1.f)  },	// 3
	};
	const unsigned short m_QuadIndexData[6] = {
		0, 1, 2, 0, 2, 3
	};

protected:

	virtual void UpdateContent(float InDeltaTime) override;

};

#endif // TexturesExample_h__