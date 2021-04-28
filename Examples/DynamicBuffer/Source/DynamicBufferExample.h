/*
 DynamicBufferExample.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef DynamicBufferExample_h__
#define DynamicBufferExample_h__

#include "Application.h"
#include "GraphicsTypes.h"
#include "MoxMath.h"

namespace Mox { class PipelineState; class Entity; }

class DynBufExampleApp : public Mox::Application
{
public:
	DynBufExampleApp();

	virtual void OnInitializeContent() override;

	virtual void OnQuitApplication() override;

private:


	// Callbacks for main window mouse and keyboard events
	void OnMouseWheel(float InDeltaRot);
	void OnMouseMove(int32_t InX, int32_t InY);
	void OnLeftMouseDrag(int32_t InDeltaX, int32_t InDeltaY);
	void OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY);
	void OnTypingKeyPressed(Mox::KEYBOARD_KEY InPressedKey);
	void OnControlKeyPressed(Mox::KEYBOARD_KEY InPressedKey);

	// Vertex buffer for the cube
	Mox::Resource* m_VertexBuffer;
	Mox::VertexBufferView* m_VertexBufferView;
	// Index buffer for the cube
	Mox::Resource* m_IndexBuffer;
	Mox::IndexBufferView* m_IndexBufferView;
	// Standalone Constant Buffer for the color modifier
	Mox::DynamicBuffer* m_ColorModBuffer;
	Mox::ConstantBufferView* m_ColorModBufferView;

	Mox::PipelineState* m_PipelineState;

	Mox::Entity* m_CubeEntity;

	Mox::Matrix4f m_MvpMatrix;

	// Vertex data for colored cube
	struct VertexPosColor
	{
		Mox::Vector3f Position;
		Mox::Vector3f Color;
	};

	const VertexPosColor m_VertexData[8] = {
		{ Mox::Vector3f(-1.f, -1.f, -1.f), Mox::Vector3f(0.f, 0.f, 0.f) }, // 0
		{ Mox::Vector3f(-1.f, 1.f, -1.f), Mox::Vector3f(0.f, 1.f, 0.f) }, // 1
		{ Mox::Vector3f(1.f, 1.f, -1.f), Mox::Vector3f(1.f, 1.f, 0.f) }, // 2
		{ Mox::Vector3f(1.f, -1.f, -1.f), Mox::Vector3f(1.f, 0.f, 0.f) }, // 3
		{ Mox::Vector3f(-1.f, -1.f, 1.f), Mox::Vector3f(0.f, 0.f, 1.f) }, // 4
		{ Mox::Vector3f(-1.f, 1.f, 1.f), Mox::Vector3f(0.f, 1.f, 1.f) }, // 5
		{ Mox::Vector3f(1.f, 1.f, 1.f), Mox::Vector3f(1.f, 1.f, 1.f) }, // 6
		{ Mox::Vector3f(1.f, -1.f, 1.f), Mox::Vector3f(1.f, 0.f, 1.f) }  // 7
	};

	const unsigned short m_IndexData[36] = {
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

protected:

	virtual void UpdateContent(float InDeltaTime) override;

	virtual void RenderMainView(Mox::CommandList& InCmdList, const Mox::ContextView& InMainView) override;



};

#endif // DynamicBufferExample_h__