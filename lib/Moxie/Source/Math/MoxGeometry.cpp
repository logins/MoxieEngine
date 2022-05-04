/*
 Geometry.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "MoxGeometry.h"

namespace Mox {

	// Note: Direct3D and OpenGL BOTH set the Matrix Packing Order for Uniform Parameters to Column-Major.
	// https://docs.microsoft.com/en-gb/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math?redirectedfrom=MSDN
	//
	// According to this source: https://antiagainst.github.io/post/hlsl-for-vulkan-matrices/#hlsl-matrices
	// HLSL will change the majorness if requested, ONLY for external data, the one read from GPU memory (so the one uploaded by our program), 
	// while the local variables will be always stored as Row Major.
	// Majorness only matters for external initialized matrices, 
	// because it controls how they transform from the storage to the mathematical form.
	//
	// The difference is: HLSL (D3D shader language) and DXMATH (DX Math Library) identify vectors as Row Vectors.
	//
	// Heigen, glm and Spir-V libraries store data in Column-Major by default! And every Vector is a Column by default.
	//
	// That does NOT impact matrix multiplication oder of operand.
	//
	// Instead, that DOES impact how we do algebra operations in our code, 
	// such as the vector Base Change operation: with Eigen, every change of base will need to be 
	// V1 = M V0 where M is the base change matrix, V0 the initial vector and V1 the vector in the new base.
	// That also means, Model(Mm) View(Mv) Projection(Mp) base change will need to be acted as: V1 = Mp Mv Mm V0.
	//

	Mox::Matrix4f Perspective(float InZNear, float InZFar, float InAspectRatio, float InFovYRad)
	{
		// FROM: https://docs.microsoft.com/en-us/windows/win32/direct3d9/projection-transform
		// Note: Using the Perspective and LookAt from OpenGL in D3D will render the back faces !!!
		Mox::ProjectiveTransform<float, 3> tr;
		tr.matrix().setZero();
		assert(InZFar > InZNear);
		float h = 1.0f / std::tan(InFovYRad / 2.f);
		float w = 1.0f / (InAspectRatio * std::tan(InFovYRad / 2.f));
		float Q = InZFar / (InZFar - InZNear);
		tr(0, 0) = w;
		tr(1, 1) = h;
		tr(2, 2) = Q;
		tr(3, 2) = 1.0f;
		tr(2, 3) = -Q * InZNear;

		return tr.matrix();
	}

	Mox::Matrix4f LookAt(const Mox::Vector3f& eye, const Mox::Vector3f& center, const Mox::Vector3f& up)
	{
		// FROM: https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		// Note: Using the Perspective and LookAt from OpenGL in D3D will render the back faces !!!
		//typedef Eigen::Matrix<float, 4, 4> Matrix4;
		//typedef Eigen::Matrix<float, 3, 1> Vector3;
		Mox::Matrix4f mat = Mox::Matrix4f::Zero();
		Mox::Vector3f zAxis = (center - eye).normalized();
		Mox::Vector3f xAxis = up.cross(zAxis).normalized();
		Mox::Vector3f yAxis = zAxis.cross(xAxis).normalized();
		mat(0, 0) = xAxis.x();
		mat(0, 1) = xAxis.y();
		mat(0, 2) = xAxis.z();
		mat(0, 3) = -xAxis.dot(eye);
		mat(1, 0) = yAxis.x();
		mat(1, 1) = yAxis.y();
		mat(1, 2) = yAxis.z();
		mat(1, 3) = -yAxis.dot(eye);
		mat(2, 0) = zAxis.x();
		mat(2, 1) = zAxis.y();
		mat(2, 2) = zAxis.z();
		mat(2, 3) = -zAxis.dot(eye);
		mat.row(3) << 0, 0, 0, 1;
		return mat;
	}

	// Note: before these functions were templated, with an explicit template instantiation.
	// Explicit template instantiation, this makes possible defining templated function bodies in .cpp files
	// https://stackoverflow.com/questions/115703/storing-c-template-function-definitions-in-a-cpp-file
	// The downside is that we need to instantiate the template for each type we need.

	Mox::Matrix4f ModelMatrix(const Vector3i& InPosition, const Vector3f& InScale, const Vector3f& InAngleRotation)
	{
		// Generate Rotation matrix
		// Good article about rotation matrix https://www.continuummechanics.org/rotationmatrix.html
		// and also Wolphram https://mathworld.wolfram.com/EulerAngles.html
		/*
		* --- Considering Z to be the axis that points to the top ---
		psi: angle rotation on the Z-axis
		theta: angle rotation on Y after psi applied
		phi: a second rotation on the Z-axis after theta applied

		*/
		Mox::Vector3f radianRotation = InAngleRotation * EIGEN_PI / 180.f;

		Mox::Vector3f sinAxis(std::sin(radianRotation.x()), std::sin(radianRotation.y()), std::sin(radianRotation.z()));
		Mox::Vector3f cosAxis(std::cos(radianRotation.x()), std::cos(radianRotation.y()), std::cos(radianRotation.z()));
		
		Mox::Matrix3f rotScaleMat; rotScaleMat << cosAxis.x(), -sinAxis.x(), 0.f, sinAxis.x(), cosAxis.x(), 0.f, 0.f, 0.f, 1.f;
		Mox::Matrix3f rotPartial; rotPartial << cosAxis.y(), 0.f, sinAxis.y(), 0.f, 1.f, 0.f, -sinAxis.y(), 0.f, cosAxis.y();

		rotScaleMat *= rotPartial;

		rotPartial << cosAxis.z(), -sinAxis.z(), 0.f, sinAxis.z(), cosAxis.z(), 0.f, 0.f, 0.f, 1.f;

		rotScaleMat *= rotPartial;

		// Generate and use Scale matrix
		Mox::Matrix3f scaleMatrix;
		scaleMatrix <<	
			InScale.x(), 0.f, 0.f,
			0.f, InScale.y(), 0.f,
			0.f, 0.f, InScale.z();

		rotScaleMat *= scaleMatrix;

		// Generate Translation matrix
		Mox::Matrix4f outMat;
		outMat <<
			0.f, 0.f, 0.f, InPosition.x(),
			0.f, 0.f, 0.f, InPosition.y(),
			0.f, 0.f, 0.f, InPosition.z(),
			0.f, 0.f, 0.f, 1.f;
		
		// Copy rotScale into the top-left 3x3 block
		outMat.block<3, 3>(0, 0) = rotScaleMat;

		return std::move(outMat);
	}

}
