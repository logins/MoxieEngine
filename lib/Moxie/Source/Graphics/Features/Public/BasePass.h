/*
 BasePass.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef BasePass_h__
#define BasePass_h__

#include "RenderPass.h"
#include "PipelineState.h"

namespace Mox {


	class RenderProxy;
	class CommandList;

	// This call is needed for all the render passes we want to register
	REGISTER_RENDER_PASS(BasePass)


	class BasePass : public RenderPass
	{

	public:
		BasePass() = default;

		virtual ~BasePass();

		void SetupPass(Mox::CommandList& InCmdList) override;

		void ProcessRenderProxy(Mox::RenderProxy& InProxy) override;

		void SendDrawCommands(Mox::CommandList& InCmdList) override;

	private:

		std::unique_ptr<Mox::PipelineState::GRAPHICS_PSO_DESC> m_DefaultPSODesc;
		
	};
}
#endif // BasePass_h__