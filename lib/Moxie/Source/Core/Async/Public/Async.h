/*
 Async.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Async_h__
#define Async_h__

namespace Mox {

class SystemThread {

	virtual void Run() = 0;

};

}

#endif // Async_h__