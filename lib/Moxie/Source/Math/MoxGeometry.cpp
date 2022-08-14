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

	Mox::Matrix4f ModelMatrix(const Vector3f& InPosition, const Vector3f& InScale, const Vector3f& InAngleRotation)
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

	void AddTriangleIndices(std::vector<uint16_t>& InIndices, uint16_t InA, uint16_t InB, uint16_t InC)
	{
		// This function can be used to reverse the oder of which the indices get inserted
		InIndices.emplace_back(InA);
		InIndices.emplace_back(InB);
		InIndices.emplace_back(InC);
	}

	/*
	Indices are meant to be passed for a quad with vertices at:
	InA --- InB
	|		|
	InD --- InC
	*/
	void AddQuadIndices(std::vector<uint16_t>& InIndices, uint16_t InA, uint16_t InB, uint16_t InC, uint16_t InD)
	{
		AddTriangleIndices(InIndices, InA, InB, InC);
		AddTriangleIndices(InIndices, InA, InC, InD);
	}

	void AddQuadIndicesAlt(std::vector<uint16_t>& InIndices, uint16_t InA, uint16_t InB, uint16_t InC, uint16_t InD)
	{
		AddTriangleIndices(InIndices, InA, InB, InD);
		AddTriangleIndices(InIndices, InB, InC, InD);
	}

	Mox::Vector2f Sphere2MapUvEquirectangular(const Mox::Vector3f& InPos)
	{
		return Mox::Vector2f(
			std::atan2(InPos.x(), -InPos.z()) / (2.f * EIGEN_PI) + .5f,
			InPos.y() * .5f + .5f
		);
	}

	std::tuple<double, double> Sphere2MapUvEqualArea(double Inx, double Iny, double Inz)
	{
		return {
			(std::atan2(Inx, -Inz) / EIGEN_PI + 1) / 2,
			std::asin(Iny) / EIGEN_PI + .5
		};
	}

	std::tuple<double,double> Sphere2MapUv(double Inx, double Iny, double Inz)
	{
		//return Sphere2MapUvEquirectangular(InPos);
		
		return Sphere2MapUvEqualArea(Inx, Iny, Inz);
	}
#undef max
#undef min

	// Meridians = vertical
	// Parallels = horizontal
	// Note: With UV Sphere algorithm, we are going to have very slim triangle primitives to the poles, which will increase the 
	// texture aliasing in those areas. In general, mapping a 2D texture map to a sphere will always generate texture artifacts in the poles area,
	// but UV sphere will make them more evident. In general, this is not a sphere generation algorithm that is used in production, both for the
	// texture artifacts but also because of the big delta in approximating a spheric shape.
	// A Cube-To-Sphere generation algorithm is usually to prefer to this one.
	// You can read more about it at:
	// https://github.com/caosdoar/spheres
	void UVSphere(uint32_t InMeridians, uint32_t InParallels, std::vector<Mox::Vector3f>& OutMeshVertices, std::vector<Mox::Vector2f>& OutMeshUvs, std::vector<uint16_t>& OutMeshIndices)
	{
		// Following code has been found at:
		//https://github.com/caosdoar/spheres/blob/master/src/spheres.cpp

		double curU, curV;

		// Placing vertex at the top first
		OutMeshVertices.emplace_back(0.0f, 1.0f, 0.0f);
		OutMeshUvs.emplace_back(0.5f, 1.f);

		// For each horizontal subdivision (Parallels)
		for (uint32_t j = 0; j < InParallels; ++j)
		{
			// polesPolarSize is the vertical angle reserved for every pole. The rest of the angle will be equally subdivided among the parallels.
			static constexpr double polesPolarSize = EIGEN_PI * (1.f / 15.f);
			double const polar = polesPolarSize + ((EIGEN_PI - 2*polesPolarSize) * double(j) / double(InParallels-1));
			double const sp = std::sin(polar);
			double const cp = std::cos(polar);

			// Generate a horizontal ring of vertices
			for (uint32_t i = 0; i < InMeridians; ++i)
			{
				// half-Pi rotation will make the ring generation to align with texture coords starting with u=0
				// Due to precision issues, starting with 0.5*PI would make u start from 1 and immediately switch back
				// to 0. Instead we want the texture wrapping to start from 0, so we consider a value slightly higher.
				static constexpr double halfPi = 0.5000000001 * EIGEN_PI;
				double azimuth = halfPi + (2.0 * EIGEN_PI * double(i) / double(InMeridians));
				double const sa = std::sin(azimuth);
				double const ca = std::cos(azimuth);
				double const x = sp * ca;
				double const y = cp;
				double const z = sp * sa;
				OutMeshVertices.emplace_back(x, y, z);
				auto [curU, curV] = Sphere2MapUv(x, y, z);
				OutMeshUvs.emplace_back(curU, curV);
			}
			// At the end of the vertex loop adding a vertex with the same position as the first
			// but having uv coordinates with u = 1.
			// This will prevent texture interpolation to make wrong "wrap-around" between 
			// values close to u=0.9 and u=0.1
			OutMeshVertices.emplace_back(OutMeshVertices[1 + j * (InMeridians+1)]);
			Mox::Vector2f oldUv = OutMeshUvs[1 + j * (InMeridians+1)];
			oldUv.x() = 1.f;
			OutMeshUvs.emplace_back(std::move(oldUv));
		}
		
		// Placing vertex at the bottom last
		OutMeshVertices.emplace_back(0.0f, -1.0f, 0.0f);
		OutMeshUvs.emplace_back(0.5f, 0.f);

		// Building the triangle fan at the top (traingles sharing vertex in pos [0,1,0])
		for (uint32_t i = 0; i < InMeridians; ++i)
		{
			uint32_t const a = i + 1; // +1 because we pass beyond the vertex at the top of the sphere
			uint32_t const b = i + 2;
			AddTriangleIndices(OutMeshIndices,0, b, a);
		}

		// Number of vertices per each parallel is number of meridians (vertical slices) + 1
		// because we inserted the additional vertex to adjust UVs
		uint32_t parallelVertexNum = InMeridians + 1;

		// Note: vertices were stored in parallel-major
		for (uint32_t j = 0; j < InParallels-1; ++j)
		{
			uint32_t aStart = 1 + j * parallelVertexNum; // +1 because we are jumping the first vertex which is the one at the top of the sphere
			uint32_t bStart = 1 + (j + 1) * parallelVertexNum; // vertex bStart is below aStart
			// Generating quads on the horizontal band between parallel j and j+1
			for (uint32_t i = 0; i < InMeridians; ++i)
			{
				const uint32_t a = aStart + i;
				const uint32_t a1 = aStart + i + 1;
				const uint32_t b = bStart + i;
				const uint32_t b1 = bStart + i + 1;
				AddQuadIndices(OutMeshIndices, a, a1, b1, b);
			}
		}
		
		// Building the triangle fan at the bottom (traingles sharing vertex in pos [0,-1,0])
		for (uint32_t i = 0; i < InMeridians; ++i)
		{
			uint32_t const a = 1 + parallelVertexNum * (InParallels - 1) + i;
			uint32_t const b = 1 + parallelVertexNum * (InParallels - 1) + (i + 1);
			AddTriangleIndices(OutMeshIndices, OutMeshVertices.size() - 1, a, b);
		}
		
	}


	namespace CubeToSphere
	{
		static const Mox::Vector3f origins[6] =
		{
			Mox::Vector3f(-1.0, -1.0, -1.0),
			Mox::Vector3f(1.0, -1.0, -1.0),
			Mox::Vector3f(1.0, -1.0, 1.0),
			Mox::Vector3f(-1.0, -1.0, 1.0),
			Mox::Vector3f(-1.0, 1.0, -1.0),
			Mox::Vector3f(-1.0, -1.0, 1.0)
		};
		static const Mox::Vector3f rights[6] =
		{
			Mox::Vector3f(2.0, 0.0, 0.0),
			Mox::Vector3f(0.0, 0.0, 2.0),
			Mox::Vector3f(-2.0, 0.0, 0.0),
			Mox::Vector3f(0.0, 0.0, -2.0),
			Mox::Vector3f(2.0, 0.0, 0.0),
			Mox::Vector3f(2.0, 0.0, 0.0)
		};
		static const Mox::Vector3f ups[6] =
		{
			Mox::Vector3f(0.0, 2.0, 0.0),
			Mox::Vector3f(0.0, 2.0, 0.0),
			Mox::Vector3f(0.0, 2.0, 0.0),
			Mox::Vector3f(0.0, 2.0, 0.0),
			Mox::Vector3f(0.0, 0.0, 2.0),
			Mox::Vector3f(0.0, 0.0, -2.0)
		};
	};

	void NormalizedCube(uint32_t InDivisions, std::vector<Mox::Vector3f>& OutMeshVertices, std::vector<Mox::Vector2f>& OutMeshUvs, std::vector<uint16_t>& OutMeshIndices)
	{
		const float step = 1.0 / float(InDivisions);
		const Mox::Vector3f step3(step, step, step);

		for (uint32_t face = 0; face < 6; ++face)
		{
			const Mox::Vector3f origin = CubeToSphere::origins[face];
			const Mox::Vector3f right = CubeToSphere::rights[face];
			const Mox::Vector3f up = CubeToSphere::ups[face];
			for (uint32_t j = 0; j < InDivisions + 1; ++j)
			{
				float fj = static_cast<float>(j);
				const Mox::Vector3f j3(fj, fj, fj);
				for (uint32_t i = 0; i < InDivisions + 1; ++i)
				{
					float fi = static_cast<float>(i);
					const Mox::Vector3f i3(fi, fi, fi);
					const Mox::Vector3f p = origin + step3.cwiseProduct(i3.cwiseProduct(right) + j3.cwiseProduct(up)); //step3 * (i3 * right + j3 * up);
					OutMeshVertices.emplace_back(p.normalized().x(), p.normalized().y(), p.normalized().z());
					std::tuple<double,double> curUvs = Sphere2MapUv(p.normalized().x(), p.normalized().y(), p.normalized().z());
					
					OutMeshUvs.emplace_back(std::get<0>(curUvs), std::get<1>(curUvs));
				}
			}
		}

		const uint32_t k = InDivisions + 1;
		for (uint32_t face = 0; face < 6; ++face)
		{
			for (uint32_t j = 0; j < InDivisions; ++j)
			{
				const bool bottom = j < (InDivisions / 2);
				for (uint32_t i = 0; i < InDivisions; ++i)
				{
					const bool left = i < (InDivisions / 2);
					const uint32_t a = (face * k + j) * k + i;
					const uint32_t b = (face * k + j) * k + i + 1;
					const uint32_t c = (face * k + j + 1) * k + i;
					const uint32_t d = (face * k + j + 1) * k + i + 1;
					if (bottom ^ left) AddQuadIndicesAlt(OutMeshIndices, a, c, d, b);
					else AddQuadIndices(OutMeshIndices, a, c, d, b);
				}
			}
		}
	}

}
