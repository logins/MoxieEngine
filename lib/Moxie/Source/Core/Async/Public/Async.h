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

	// Note: reading this variable from other threads is usually subject to race conditions and it is used as a generic indicator only
	inline float GetCurrentFrameTime() const { return m_DeltaTime; }

protected:
	float m_DeltaTime = 0.f;
};

}

#endif // Async_h__