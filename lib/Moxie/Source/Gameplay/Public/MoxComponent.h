/*
 MoxComponent.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef MoxComponent_h__
#define MoxComponent_h__

#include "MoxMath.h"

namespace Mox {

	

// Element that refers to an Entity
class Component
{
public:
	// Callback for reacting to Entity possession
	virtual void OnPossessedBy(class Mox::Entity& InEntity) = 0;

	virtual void OnEntityTransformChanged(const Mox::Matrix4f& InNewModelMat) = 0;

	virtual ~Component();
};

}

#endif // MoxComponent_h__
