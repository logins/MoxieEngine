/*
 MoxRenderProxy.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef MoxRenderProxy_h__
#define MoxRenderProxy_h__

namespace Mox {

	struct VertexBufferView;
	struct IndexBufferView;
	struct ConstantBufferView;
	class Mesh;

	/*
		RenderProxy: Render thread representation of a Mox::Entity in a Mox::World.
		It holds a collection of Mox::Mesh objects (with a vertex and index buffers, and shader parameters).
	*/
	class RenderProxy
	{
	public:
		RenderProxy(const std::vector<Mox::Mesh*> InMeshes);


		std::vector<Mox::Mesh*> m_Meshes;
	};


	// Used by the simulation thread to transfer object changes to the render thread
	struct FrameRenderUpdates
	{
		std::vector<Mox::RenderProxy*> m_NewProxies;

		std::vector<ConstantBufferUpdate> m_ConstantUpdates;

	};
}
#endif // MoxRenderProxy_h__