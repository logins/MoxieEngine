/*
 CommandQueue.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef CommandQueue_h__
#define CommandQueue_h__

namespace Mox { 

enum class COMMAND_LIST_TYPE : int;

class Device;
class CommandList;

/*!
 * \class CommandQueue
 *
 * \brief Platform agnostic representation of a graphics command queue.
 *
 * \author Riccardo Loggini
 * \date July 2020
 */
	class CommandQueue
	{
	public:
		virtual ~CommandQueue() = default;


		virtual Mox::CommandList& GetAvailableCommandList() = 0;

		virtual uint64_t ExecuteCmdList(Mox::CommandList& InCmdList) = 0;

		virtual void Flush() = 0;

		virtual void OnRenderFrameStarted() = 0;

		virtual void OnRenderFrameFinished() = 0;

		virtual uint64_t ComputeFramesInFlightNum() = 0;

		virtual void WaitForQueuedFramesOnGpu(uint64_t InFramesToWaitNum) = 0;

	};



}

#endif // CommandQueue_h__
