// Copyright 2015 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#pragma once

namespace Genesis
{
	// The following constants are float variants copied from the ones available from Windows' math.h when _USE_MATH_DEFINES is declared.
	// They are made available here to make it cross-platform.
	static const float kE			= 2.71828182845904523536f;
	static const float kLog2E		= 1.44269504088896340736f;
	static const float kLog10E		= 0.434294481903251827651f;
	static const float kLn2			= 0.693147180559945309417f;
	static const float kLn10		= 2.30258509299404568402f;
	static const float kPi			= 3.14159265358979323846f;
	static const float kPi2			= 1.57079632679489661923f;
	static const float kPi4			= 0.785398163397448309616f;
	static const float k1Pi			= 0.318309886183790671538f;
	static const float k2Pi			= 0.636619772367581343076f;
	static const float k2SqrtPi		= 1.12837916709551257390f;
	static const float kSqrt2		= 1.41421356237309504880f;
	static const float kSqrt12		= 0.707106781186547524401f;

	// Degrees to radians and vice-versa
	static const float kDegToRad	= kPi / 180.0f;
	static const float kRadToDeg	= 180.0f / kPi;
}
