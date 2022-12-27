/*
 MoxieLogoScene.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef MoxieLogoScene__h_
#define MoxieLogoScene__h_

#include "Application.h"


	class MoxieLogoSceneApp : public Mox::Application
	{
	public:
		MoxieLogoSceneApp() = default;

		virtual void OnInitializeContent() override;
	private:


		struct TexVertexType {
			Mox::Vector3f Position;
			Mox::Vector3f TexCoords;
		};
		const Mox::INPUT_LAYOUT_DESC m_VertexLayoutDesc = {
			{
				{"POSITION",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
				{"TEXCOORD",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
			}
		};

		// ----- SkyDome-related data -----

		Mox::VertexBuffer* m_SkydomeVertexBuffer;
		Mox::IndexBuffer* m_SkydomeIndexBuffer;
		std::unique_ptr<Mox::Texture> m_SkydomeCubeTexture;

		Mox::Entity* m_SkydomeEntity;

		// ----- Sphere-related data -----

		Mox::VertexBuffer* m_SphereVertexBuffer;
		Mox::IndexBuffer* m_SphereIndexBuffer;
		std::unique_ptr<Mox::Texture> m_SphereCubeTexture;

		Mox::Entity* m_SphereEntity;

		// ----- Quad-related data -----

		const TexVertexType m_QuadVertexData[4] =
		{
			{ Mox::Vector3f(-1.f, -0.5f, 0.f),	Mox::Vector3f(0.f, 1.f, 0.f)  },	// 0
			{ Mox::Vector3f(-1.f, 1.f, 0.f),	Mox::Vector3f(0.f, 0.f, 0.f)  },	// 1
			{ Mox::Vector3f(1.f, 1.f, 0.f),		Mox::Vector3f(1.f, 0.f, 0.f)  },	// 2
			{ Mox::Vector3f(1.f, -0.5f, 0.f),	Mox::Vector3f(1.f, 1.f, 0.f)  },	// 3
		};
		const unsigned short m_QuadIndexData[6] = {
			0, 1, 2, 0, 2, 3
		};

		Mox::VertexBuffer* m_QuadVertexBuffer;
		Mox::IndexBuffer* m_QuadIndexBuffer;
		std::unique_ptr<Mox::Texture> m_QuadTexture;

		Mox::Entity* m_QuadEntity;

	protected:

		virtual void UpdateContent(float InDeltaTime) override;

	};


#endif // MoxieLogoScene__h_