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

	// ----- Sphere-related data -----

	Mox::VertexBuffer* m_SphereVertexBuffer;
	Mox::IndexBuffer* m_SphereIndexBuffer;
	std::unique_ptr<Mox::Texture> m_SphereCubeTexture;

	Mox::Entity* m_QuadEntity;

	struct QuadVertex {
		Mox::Vector3f Position;
		Mox::Vector3f TexCoords;
	};
	const Mox::INPUT_LAYOUT_DESC m_SphereVertexLayoutDesc = {
		{
			{"POSITION",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
			{"TEXCOORD",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
		}
	};
	const QuadVertex m_SphereVertexData[4] = 
	{
		{ Mox::Vector3f(-1.f, -1.f, 0.f),	Mox::Vector3f(0.f, 1.f, 0.f)  },	// 0
		{ Mox::Vector3f(-1.f, 1.f, 0.f),	Mox::Vector3f(0.f, 0.f, 0.f)  },	// 1
		{ Mox::Vector3f(1.f, 1.f, 0.f),		Mox::Vector3f(1.f, 0.f, 0.f)  },	// 2
		{ Mox::Vector3f(1.f, -1.f, 0.f),	Mox::Vector3f(1.f, 1.f, 0.f)  },	// 3
	}
	;
	const unsigned short m_QuadIndexData[6] = {
		0, 1, 2, 0, 2, 3
	};

protected:

	virtual void UpdateContent(float InDeltaTime) override;

};

#endif // TexturesExample_h__