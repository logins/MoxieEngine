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

	Mox::Matrix4f ModelMatrix(const Vector3f& InPosition, const Vector3f& InScale, const Vector3f& InAngleRotation);

	// Generates a sphere mesh
	// Meridians is the number of vertical slices
	// Parallels is the number of horizontal subdivisions
	void UVSphere(uint32_t InMeridians, uint32_t InParallels, std::vector<Mox::Vector3f>& OutMeshVertices, std::vector<Mox::Vector2f>& OutMeshUvs, std::vector<uint16_t>& OutMeshIndices);

	void NormalizedCube(uint32_t InDivisions, std::vector<Mox::Vector3f>& OutMeshVertices, std::vector<Mox::Vector2f>& OutMeshUvs, std::vector<uint16_t>& OutMeshIndices);

}
#endif // Geometry_h__