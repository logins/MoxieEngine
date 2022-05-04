/*
 RenderPass.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef RenderPass_h__
#define RenderPass_h__
#include "DrawCommand.h"

namespace Mox {

	class RenderProxy;
	class CommandList;


	using RenderPassVector = std::vector<std::unique_ptr<class RenderPass>>;


	/*
	RenderPass abstracts a set of draw calls or dispatches with similar intent.
	Its duty is process render proxy and generate draw commands from them, 
	that later will be translated and sent to the GPU by the render thread.
	*/
	class RenderPass
	{
	public:

		RenderPass() = default; // Note: this is needed to not delete constructor at compile time, which would prevent unique_ptr to work with this class

		virtual ~RenderPass() = default; // Note: this is needed to not delete the destructor of this class at compile time. 
		// Deleting destructor of this class would bring failure when using std::unique_ptr<RenderPass>

		//RenderPass(const RenderPass&) = delete;
		//RenderPass& operator=(const RenderPass&) = delete;

		// Sets up pipeline state, including root signature and shaders
		// (for simplicity, at the moment, there is one static PSO for render pass) 
		virtual void SetupPass() = 0;

		// Inspects relevant parameters of the proxy: if relevant generates a draw command out from it.
		virtual void ProcessRenderProxy(Mox::RenderProxy& InProxy) = 0;

		virtual void SendDrawCommands(Mox::CommandList& InCmdList) = 0;

	

		// Used to retrieve render passes during global scope variables initialization

		static RenderPassVector& GetRegisteredRenderPasses();

		// Each render pass that we want to register will need to have 
		// a static boolean variable initialized calling this function
		template <typename T, typename... VariadicTypes>
		static bool RegisterRenderPass(VariadicTypes... InVariadiArgs)
		{
			GetRegisteredRenderPasses().push_back(std::make_unique<T>(InVariadiArgs...));
			return true;
		}

	protected:
		
		// Note: if this was an unordered set, we would have to define the hashing function for Mox::DrawCommand.
		// In our case, what we usually do with draw commands is iterating all of them every time, so a vector is enough.
		std::vector<Mox::DrawCommand> m_DrawCommands; 

		Mox::PipelineState* m_PipelineState;

	};

	// Utility macro to register render passes. It needs to be called for each render pass we want to register, 
	// it can be put at the start of the implementation file
#define REGISTER_RENDER_PASS(InClassName) \
	static const bool g_InClassNameRegistered = Mox::RenderPass::RegisterRenderPass<class InClassName>();


	// Helper function to retrieve registered render passes
	static RenderPassVector& GetRenderPasses() { return Mox::RenderPass::GetRegisteredRenderPasses(); }

}
#endif // RenderPass_h__