// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef GEOMETRYTOOLS_H__
#define GEOMETRYTOOLS_H__



#include "tgt/vector.h"

namespace campvis {

    /**
     * Implements various algorithms from algorithmic geometry 
     */
    struct AlgorithmicGeometry {
    public:
        /**
         * Computes the orientation2D of the three points a, b, c in 2D.
         * \param   a   First vertex
         * \param   b   Second vertex
         * \param   c   Third vertex
         * \return  Left turn: &lt; 0, collinear: = 0, right turn: &gt; 0
         */
        template<typename T>
        static T orientation2D(const tgt::Vector2<T>& a, const tgt::Vector2<T>& b, const tgt::Vector2<T>& c) {
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
        static bool collinear2D(const tgt::Vector2<T>& a, const tgt::Vector2<T>& b, const tgt::Vector2<T>& c) {
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
        static bool leftTurn2D(const tgt::Vector2<T>& a, const tgt::Vector2<T>& b, const tgt::Vector2<T>& c) {
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
        static bool rightTurn2D(const tgt::Vector2<T>& a, const tgt::Vector2<T>& b, const tgt::Vector2<T>& c) {
            return (orientation2D(a, b, c) < 0);
        };
    };

}

#endif // GEOMETRYTOOLS_H__
