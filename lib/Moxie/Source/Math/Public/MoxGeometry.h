/*
 Geometry.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Geometry_h__
#define Geometry_h__

#include "MoxMath.h"
#include <Eigen/Geometry>


namespace Mox {

	using AngleAxisf = typename Eigen::AngleAxisf;

	Mox::Matrix4f Perspective(float InZNear, float InZFar, float InAspectRatio, float InFovYRad);

	Mox::Matrix4f LookAt(const Mox::Vector3f& InEye, const Mox::Vector3f& InCenter, const Mox::Vector3f& InUp);

}
#endif // Geometry_h__