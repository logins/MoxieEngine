/*
 RenderPass.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "RenderPass.h"

Mox::RenderPassVector& Mox::RenderPass::GetRegisteredRenderPasses()
{
	// Container for registered render passes, implemented with Registar pattern https://stackoverflow.com/questions/10589779/enumerating-derived-classes-in-c-executable
	static RenderPassVector RegisteredRenderPasses;

	return RegisteredRenderPasses;
}

