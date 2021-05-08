/*
 Async.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Async_h__
#define Async_h__

namespace Mox {

class SystemThread {
public:
	virtual void Run() = 0;

	void Join() { if(m_InnerThread) m_InnerThread->join(); }

	// Note: reading this variable from other threads is usually subject to race conditions and it is used as a generic indicator only
	inline float GetCurrentFrameTime() const { return m_DeltaTime; }

protected:

	std::unique_ptr<std::thread> m_InnerThread;

	float m_DeltaTime = 0.f;

};

}

#endif // Async_h__