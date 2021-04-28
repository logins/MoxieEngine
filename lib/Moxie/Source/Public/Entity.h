/*
 Entity.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Entity_h__
#define Entity_h__

namespace Mox {

// Object seen by the simulator
class Entity
{
public:
	// Position of the entity in the scene
	Mox::Matrix4f m_ModelMatrix;
};

}

#endif // Entity_h__