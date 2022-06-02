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
	class Drawable;

	/*
		RenderProxy: Render thread representation of a Mox::Entity in a Mox::World.
		It holds a collection of Mox::Mesh objects (with a vertex and index buffers, and shader parameters).
	*/
	class RenderProxy
	{
	public:
		RenderProxy();

		~RenderProxy();
		RenderProxy(RenderProxy&&) noexcept;

		void AddDrawable(Mox::Drawable* InDrawable) { m_Meshes.push_back(InDrawable); }

		std::vector<Mox::Drawable*> m_Meshes;
	};

}
#endif // MoxRenderProxy_h__