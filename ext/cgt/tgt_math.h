/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_MATH_H
#define TGT_MATH_H

#include "cgt/assert.h"
#include "cgt/types.h"

#include <cmath>
#include <cfloat>

#include <algorithm>

/*
    work around for windows.h
*/
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace tgt {

/*
    constant pi
*/

#ifdef PI
#undef PI
#endif

const double PI = 3.14159265358979323846;
const float PIf = 3.14159265358979323846f;

/*
    conversion from degree to radian and vice versa
*/

/// convert a radian to degree
inline float rad2deg(float f) {
    return (180.f / PIf) * f;
}

/// convert a radian to degree
inline double rad2deg(double d) {
    return (180.0 / PI) * d;
}

/// convert a degree to radian
inline float deg2rad(float f) {
    return (PIf / 180.f) * f;
}

/// convert a degree to radian
inline double deg2rad(double d) {
    return (PI / 180.0) * d;
}

/*
    sign
*/

inline float sign(float f) {
    return f == 0.f ? 0.f : f / std::abs(f);
}

inline double sign(double d) {
    return d == 0.0 ? 0.0 : d / std::abs(d);
}

inline int sign(int i) {
    return i == 0 ? 0 : i / std::abs(i);
}

inline int isign(float f) {
    return static_cast<int>(sign(f));
}

inline int isign(double d) {
    return static_cast<int>(sign(d));
}

/*
    floor
*/

// float floor(float f) defined in std c++
// double floor(double d) defined in std c++

/// Return the largest integer not greater than \p f.
inline int ifloor(float f) {
    return static_cast<int>(std::floor(f));
}

/// Return the largest integer not greater than \p d.
inline int ifloor(double d) {
    return static_cast<int>(std::floor(d));
}

/*
    ceil
*/

// float ceil(float f) defined in std c++
// double ceil(double d) defined in std c++

/// Return the smallest integer not less than \p f.
inline int iceil(float f) {
    return static_cast<int>(std::ceil(f));
}

/// Return the smallest integer not less than \p d.
inline int iceil(double d) {
    return static_cast<int>(std::ceil(d));
}

/*
    fix
*/

/// Truncate \p f towards zero.
inline float fix(float f) {
    return std::abs(f) * sign(f);
}

/// Truncate \p d towards zero.
inline double fix(double d) {
    return std::abs(d) * sign(d);
}

/// Truncate \p f towards zero.
inline int ifix(float f) {
    return static_cast<int>(fix(f));
}

/// Truncate \p d towards zero.
inline int ifix(double d) {
    return static_cast<int>(fix(d));
}

/*
    round
*/

/// Return the integer nearest to \p f.
inline float round(float f) {
    return std::floor(f + 0.5f);
}

/// Return the integer nearest to \p d.
inline double round(double d) {
    return std::floor(d + 0.5);
}

/// Return the integer nearest to \p f.
inline int iround(float f) {
    return static_cast<int>(round(f));
}

/// Return the integer nearest to \p d.
inline int iround(double d) {
    return static_cast<int>(round(d));
}

/*
    clamp
*/

/// Clamps \p f to range [\p min, \p max].
inline float clamp(float f, float min, float max) {
    return std::min( std::max(f,min), max );
}

/// Clamps \p f to range [\p min, \p max].
inline double clamp(double f, double min, double max) {
    return std::min(std::max(f, min), max);
}

/// Clamps \p f to range [\p min, \p max].
inline int clamp(int f, int min, int max) {
    return std::min(std::max(f, min), max);
}

/*
    floating point checks
 */

/// Returns false, if \p f is not a number (NaN).
inline bool isNumber(float f) {
    // according to the IEEE-754 floating-point standard
    // comparisons with NaN always fail
    return f == f;
}

/// Returns false, if \p d is not a number (NaN).
inline bool isNumber(double d) {
    // according to the IEEE-754 floating-point standard
    // comparisons with NaN always fail
    return d == d;
}

/// Returns true, if \p value with the type T is not a number (NaN).
template<typename T>
inline bool isNaN(const T& value) {
    return (value != value);
};

/// Returns false, if \p f is NaN or infinity.
inline bool isFiniteNumber(float f) {
    return (f <= FLT_MAX && f >= -FLT_MAX);
}

/// Returns false, if \p d is NaN or infinity.
inline bool isFiniteNumber(double d) {
    return (d <= DBL_MAX && d >= -DBL_MAX);
}

} // namespace tgt

#endif // TGT_MATH_H
