/*
 DrawCommand.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef DrawCommand_h__
#define DrawCommand_h__


namespace Mox {

	class PipelineState;
	struct IndexBufferView;
	struct VertexBufferView;
	struct ConstantBufferView;

	using CbvEntry = std::tuple<uint32_t, Mox::ConstantBufferView*>;
	using SrvEntry = std::tuple<uint32_t, Mox::ShaderResourceView*>;

	/*
	DrawCommand abstracts a single draw call for a single object on a single render pass.
	It will be generated and stored in the render pass, and updated among render proxy or material updates.
	*/
	struct DrawCommand
	{
		DrawCommand(Mox::VertexBufferView& InVb, Mox::IndexBufferView& InIb, Mox::PipelineState& InPs, 
			std::vector<CbvEntry> InCbvs, std::vector<SrvEntry> InSrvs = std::vector<SrvEntry>())
			: m_VertexBufferView(InVb), m_IndexBufferView(InIb), m_PipelineState(InPs), 
			// Note: This is good for both lvalues and rvalues passes to InRe. 
			m_CbvResourceEntries( std::move(InCbvs)), m_SrvResourceEntries( std::move(InSrvs)) { }; 
		// The reason is:
		// - If we pass an lvalue, InRe gets COPY constructed and then moved to m_ResourceEntries
		// - If we pass an rvalue, InRe gets MOVE constructed and then moved to m_ResourceEntries
		// So either way, the new object gets constructed only once and never copied in vain.
		// More info here https://www.fluentcpp.com/2018/07/17/how-to-construct-c-objects-without-making-copies/

		Mox::VertexBufferView& m_VertexBufferView;

		Mox::IndexBufferView& m_IndexBufferView;

		Mox::PipelineState& m_PipelineState;

		std::vector<CbvEntry> m_CbvResourceEntries;

		std::vector<SrvEntry> m_SrvResourceEntries;
	};

}
#endif // DrawCommand_h__