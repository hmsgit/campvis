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

#ifndef ALGORITHMICGEOMETRY_H__
#define ALGORITHMICGEOMETRY_H__

#include "cgt/vector.h"
#include "core/coreapi.h"

namespace campvis {

    /**
     * Implements various algorithms from algorithmic geometry 
     */
    struct CAMPVIS_CORE_API AlgorithmicGeometry {
    public:
        /**
         * Computes the orientation2D of the three points a, b, c in 2D.
         * \param   a   First vertex
         * \param   b   Second vertex
         * \param   c   Third vertex
         * \return  Left turn: &lt; 0, collinear: = 0, right turn: &gt; 0
         */
        template<typename T>
        static T orientation2D(const cgt::Vector2<T>& a, const cgt::Vector2<T>& b, const cgt::Vector2<T>& c) {
            return a.x * b.y + a.y * c.x + b.x * c.y - c.x * b.y - c.y * a.x - b.x * a.y;
        };

        /**
         * Checks whether the three points are collinear.
         * \param   a   First vertex
         * \param   b   Second vertex
         * \param   c   Third vertex
         * \return  True, when a, b, c are collinear.
         */
        template<typename T>
        static bool collinear2D(const cgt::Vector2<T>& a, const cgt::Vector2<T>& b, const cgt::Vector2<T>& c) {
            return (orientation2D(a, b, c) == 0);
        };

        /**
         * Checks whether the three points build a left turn.
         * \param   a   First vertex
         * \param   b   Second vertex
         * \param   c   Third vertex
         * \return  True, when a, b, c build a left turn.
         */
        template<typename T>
        static bool leftTurn2D(const cgt::Vector2<T>& a, const cgt::Vector2<T>& b, const cgt::Vector2<T>& c) {
            return (orientation2D(a, b, c) > 0);
        };

        /**
         * Checks whether the three points build a right turn.
         * \param   a   First vertex
         * \param   b   Second vertex
         * \param   c   Third vertex
         * \return  True, when a, b, c build a right turn.
         */
        template<typename T>
        static bool rightTurn2D(const cgt::Vector2<T>& a, const cgt::Vector2<T>& b, const cgt::Vector2<T>& c) {
            return (orientation2D(a, b, c) < 0);
        };
    };

}

#endif // ALGORITHMICGEOMETRY_H__
