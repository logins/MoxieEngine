/*
 MoxMaterial.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef MoxMaterial_h__
#define MoxMaterial_h__

namespace Mox {

	// Ultimately contains a series of shader parameters 
	// which will be used by render passes.
	class Material
	{
	public:
		int m_ID;

		static Mox::Material DefaultMaterial;
	};

}

#endif // MoxMaterial_h__
