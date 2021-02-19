/*
 Math.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Math_h__
#define Math_h__

#include <memory>
#include <Eigen/Core>

// ---------- MOX MATH ----------

// Mox Math is intended to be a wrapper for an underlying math library.
// After evaluating the possible ways to make a wrapper library (class interfaces, or PIMPL or typedefs)
// I decided that it was not worth for me to go with a full wrapper and instead it was more reasonable
// to make the underlying math library as a public dependency of Moxie and create a series of typedefs
// (with the "using" keyword) aliasing the underlying math types.
// I am not a fan of the solution but the scope of this engine does not concern building a math library
// and a proper library wrapper would also add an indirection cost up to 10% of the speed, other than
// the time spent on realizing it.

namespace Mox {

		template<typename DataType, int RowsNum, int ColsNum>
		using Matrix = typename Eigen::Matrix<DataType, RowsNum, ColsNum>;

		template<typename DataType, int Dim>
		using ProjectiveTransform = typename Eigen::Transform<DataType, Dim, Eigen::Projective>;
		template<typename DataType, int Dim>
		using AffineTransform = typename Eigen::Transform<DataType, Dim, Eigen::Affine>;

		using Matrix4f = Eigen::Matrix4f;

		using Vector3f = Eigen::Vector3f;

		using Vector2f = Eigen::Vector2f;

		// Note: InAlignmentUnit must be a power of two!
		// Aligns the input size to the input alignment unit.
		// How this works is: we first add InAlignUnit - 1 to the input size and then we "clean" all the bits interested by the alignment unit.
		// Since the InAlignmentUnit is a power of 2, by negating it, all the bits interested by the alignment will be 0 and all the rest will be 1.
		// In this way, the AND operation will place 0 (clean) all the bits interested by the alignment and leave the rest unchanged.
		// Since we added InAlignUnit - 1 at the beginning, the returned size will be always greater or equal to the input size.
		inline size_t Align(size_t InSize, size_t InAlignUnit) { return (InSize + InAlignUnit - 1) & ~(InAlignUnit - 1); }

		// Gotten from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		/*uint64_t NextPowerOfTwo(uint64_t InNumber) {
			InNumber--;
			InNumber |= InNumber >> 1;
			InNumber |= InNumber >> 2;
			InNumber |= InNumber >> 4;
			InNumber |= InNumber >> 8;
			InNumber |= InNumber >> 16;
			InNumber |= InNumber >> 32;
			InNumber |= InNumber >> 64;
			InNumber |= InNumber >> 128;
			InNumber |= InNumber >> 256;
			InNumber |= InNumber >> 512;
			InNumber++;

			return InNumber;
		}*/

}

#endif // Math_h__