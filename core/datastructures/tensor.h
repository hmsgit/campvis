// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef TENSOR_H__
#define TENSOR_H__

#include "tgt/matrix.h"

namespace campvis {

    /**
     * Second order tensor of base type T
     *
     * A second order tensor is a symmetric, positive definite 3x3 matrix, though
     * can be represented by 6 values. To save memory only these 6 values are stored,
     * use according accessor functions to get a Matrix3\<T\> representation.
     *
     * In this implementation the 6 tensor values are stored in row order as upper
     * diagonal matrix, meaning
     *               Dxx  Dxy  Dxz
     *      elem  =       Dyy  Dyz  =  [Dxx, Dxy, Dxz, Dyy, Dyz, Dzz]
     *                         Dzz
     *
     * If you have differently organized data use one of the order transforming
     * factory methods.
     **/
    template<class T>
    struct Tensor2 {
        typedef T ElemType;

        enum {
            size = 6
        };

        union {
            struct {
                T Dxx;
                T Dxy;
                T Dxz;
                T Dyy;
                T Dyz;
                T Dzz;
            };

            T elem[size];
        };


        /// Default constructor
        Tensor2() {}

        /// Init all elements with the same value
        explicit Tensor2(T v) {
            for (size_t i = 0; i < size; ++i)
                elem[i] = v;
        }
        /// Init from array with equal size
        explicit Tensor2(const T* v) {
            for (size_t i = 0; i < size; ++i)
                elem[i] = v[i];
        }

        /// Init componentwisely
        Tensor2(T Dxx, T Dxy, T Dxz, T Dyy, T Dyz, T Dzz) {
            elem[0] = Dxx;
            elem[1] = Dxy;
            elem[2] = Dxz;
            elem[3] = Dyy;
            elem[4] = Dyz;
            elem[5] = Dzz;
        }

        /// Init with another Vector of another type
        template<class U>
        Tensor2(const Tensor2<U>& v) {
            for (size_t i = 0; i < v.size; ++i)
                elem[i] = T(v.elem[i]);
        }

        /// Index operator
        const T& operator [] (size_t index) const {
            return elem[index];
        }

        /// Index operator
        T& operator [] (size_t index) {
            return elem[index];
        }

        /**
         * Returns a 3x3 matrix representation of this rank-2 Tensor
         * \return  tgt::Matrix3<T>(Dxx, Dxy, Dxz, Dxy, Dyy, Dyz, Dxz, Dyz, Dzz)
         */
        tgt::Matrix3<T> getMatrix() const {
            return tgt::Matrix3<T>(Dxx, Dxy, Dxz, Dxy, Dyy, Dyz, Dxz, Dyz, Dzz);
        }

        /**
         * Creates a second order tensor from values given in row order as
         * lower diagonal matrix, meaning
         *               Dxx
         *      elem  =  Dxy  Dyy       =  [Dxx, Dxy, Dyy, Dxz, Dyz, Dzz]
         *               Dxz  Dyz  Dzz
         **/
        static Tensor2<T> createTensorFromLowerDiagonalMatrix(T Dxx, T Dxy, T Dyy, T Dxz, T Dyz, T Dzz) {
            return Tensor2(Dxx, Dxy, Dxz, Dyy, Dyz, Dzz);
        }

        /**
         * Creates a second order tensor from values given in row order as
         * lower diagonal matrix, meaning
         *               Dxx
         *      elem  =  Dxy  Dyy       =  [Dxx, Dxy, Dyy, Dxz, Dyz, Dzz]
         *               Dxz  Dyz  Dzz
         **/
        static Tensor2<T> createTensorFromLowerDiagonalMatrix(const T* elem) {
            return Tensor2(elem[0], elem[1], elem[3], elem[2], elem[4], elem[5]);
        }

        /**
         * Creates a second order tensor from values given in row order as
         * lower diagonal matrix, meaning
         *               1  4  5
         *      elem  =     2  6  =  [Dxx, Dyy, Dzz, Dxy, Dxz, Dyz]
         *                     3
         **/
        static Tensor2<T> createTensorFromDiagonalOrder(T Dxx, T Dyy, T Dzz, T Dxy, T Dxz, T Dyz) {
            return Tensor2(Dxx, Dxy, Dxz, Dyy, Dyz, Dzz);
        }

        /**
         * Creates a second order tensor from values given in row order as
         * lower diagonal matrix, meaning
         *               1  4  5
         *      elem  =     2  6  =  [Dxx, Dyy, Dzz, Dxy, Dxz, Dyz]
         *                     3
         **/
        static Tensor2<T> createTensorFromDiagonalOrder(const T* elem) {
            return Tensor2(elem[0], elem[3], elem[4], elem[1], elem[5], elem[2]);
        }


        bool operator==(const Tensor2<T>& rhs) {
            return (   (Dxx == rhs.Dxx)
                    && (Dxy == rhs.Dxy)
                    && (Dxz == rhs.Dxz)
                    && (Dyy == rhs.Dyy)
                    && (Dyz == rhs.Dyz)
                    && (Dzz == rhs.Dzz));
        }

        bool operator!=(const Tensor2<T>& rhs) {
            return (   (Dxx != rhs.Dxx)
                    || (Dxy != rhs.Dxy)
                    || (Dxz != rhs.Dxz)
                    || (Dyy != rhs.Dyy)
                    || (Dyz != rhs.Dyz)
                    || (Dzz != rhs.Dzz));
        }

        Tensor2<T> operator*(const T& rhs) {
            return Tensor2<T>(Dxx*rhs, Dxy*rhs, Dxz*rhs, Dyy*rhs, Dyz*rhs, Dzz*rhs);
        }

        Tensor2<T> operator/(const T& rhs) {
            return Tensor2<T>(Dxx/rhs, Dxy/rhs, Dxz/rhs, Dyy/rhs, Dyz/rhs, Dzz/rhs);
        }

        Tensor2<T> operator+(const Tensor2<T>& rhs) {
            return Tensor2<T>(Dxx+rhs.Dxx, Dxy+rhs.Dxy, Dxz+rhs.Dxz, Dyy+rhs.Dyy, Dyz+rhs.Dyz, Dzz+rhs.Dzz);
        }

        Tensor2<T> operator-(const Tensor2<T>& rhs) {
            return Tensor2<T>(Dxx-rhs.Dxx, Dxy-rhs.Dxy, Dxz-rhs.Dxz, Dyy-rhs.Dyy, Dyz-rhs.Dyz, Dzz-rhs.Dzz);
        }

        Tensor2<T>& operator+=(const Tensor2<T>& rhs) {
            for (size_t i = 0; i < size; ++i)
                elem[i] += rhs.elem[i];
            return *this;
        }

        Tensor2<T>& operator-=(const Tensor2<T>& rhs) {
            for (size_t i = 0; i < size; ++i)
                elem[i] -= rhs.elem[i];
            return *this;
        }

    };

}

#endif // TENSOR_H__
