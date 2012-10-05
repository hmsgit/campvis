// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
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

#ifndef TFGEOMETRY2D_H__
#define TFGEOMETRY2D_H__

#include "sigslot/sigslot.h"
#include "tgt/vector.h"
#include <vector>

namespace tgt {
    class Texture;
}

namespace TUMVis {

    /**
     * Defines a single shape for the GeometryTransferFunction class.
     * TFGeometry2D is defined by a sorted list of KeyPoints, each having a position and a color.
     */
    class TFGeometry2D {
    public:
        struct KeyPoint {
            KeyPoint(const tgt::vec2& position, const tgt::col4& color)
                : _position(position)
                , _color(color)
            {};

            tgt::vec2 _position;
            tgt::col4 _color;
        };

        /**
         * Creates a new TFGeometry2D
         * \param   Bounds  Bounds of the position of the geometry in texture coordinates.
         */
        TFGeometry2D(const std::vector<KeyPoint>& keyPoints);

        /**
         * Virtual destructor
         */
        virtual ~TFGeometry2D();

        /**
         * Returns the vector of KeyPoints.
         * \return 
         */
        std::vector<KeyPoint>& getKeyPoints();

        /**
         * Renders this transfer function geometry to the current active OpenGL context.
         * \note    Must be called from an active and valid OpenGL context.
         * \todo    Check, whether this method really belongs here (core) or better fits into
         *          an other class in the application (GUI) module.
         */
        void render() const;

        /// Signal to be emitted when this TF geometry has changed.
        sigslot::signal0<> s_changed;

        /**
         * Creates a simple quad geometry for the given interval.
         * A quad geometry consists of two KeyPoints.
         * \param   ll          Lower left corner of the interval
         * \param   ur          Upper right corner of the interval
         * \param   color       Color for geometry
         * \return  A TFGeometry2D modelling a quad with four KeyPoints.
         */
        static TFGeometry2D* createQuad(const tgt::vec2& ll, const tgt::vec2& ur, const tgt::col4& color);
    protected:
        /**
         * Sorts the key points of this TF counter-clockwise.
         * To be called every time the position of one or more KeyPoints changes.
         */
        void computeCenterAndSortKeyPoints();

        std::vector<KeyPoint> _keyPoints;       ///< vector of KeyPoints, KeyPoints are sorted by x-coordinate of the position
        KeyPoint _center;                       ///< the center (position & color) of this geometry
    };

}

#endif // TFGEOMETRY2D_H__
