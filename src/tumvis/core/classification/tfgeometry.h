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

#ifndef TFGEOMETRY_H__
#define TFGEOMETRY_H__

#include "tgt/vector.h"
#include <vector>

namespace tgt {
    class Texture;
}

namespace TUMVis {

    /**
     * Defines a single shape for the GeometryTransferFunction class.
     * TFGeometry is defined by a sorted list of KeyPoints, each having a position and a color.
     */
    class TFGeometry {
    public:
        struct KeyPoint {
            KeyPoint(float position, const tgt::col4& color)
                : _position(position)
                , _color(color)
            {};

            float _position;
            tgt::col4 _color;
        };

        /**
         * Creates a new TFGeometry
         * \param   Bounds  Bounds of the position of the geometry in texture coordinates.
         */
        TFGeometry(const std::vector<KeyPoint>& keyPoints);

        /**
         * Virtual destructor
         */
        virtual ~TFGeometry();

        /**
         * Rasterizes this transfer function geometry into the given texture
         * \param   texture     Texture to rasterize this geometry into.
         */
        void rasterize(tgt::Texture& texture) const;

        /**
         * Creates a simple quad geometry for the given interval.
         * A quad geometry consists of two KeyPoints.
         * \param   interval    Interval the geometry resides in
         * \param   leftColor   Color for left KeyPoint
         * \param   rightColor  Color for right KeyPoint
         * \return  A TFGeometry modelling a quad with two KeyPoints.
         */
        static TFGeometry* createQuad(const tgt::vec2& interval, const tgt::col4& leftColor, const tgt::vec4 rightColor);
    protected:

        std::vector<KeyPoint> _keyPoints;       ///< vector of KeyPoints, KeyPoints are sorted by x-coordinate of the position
    };

// ================================================================================================

    /**
     * Less operator for sorting KeyPoints by their position.
     * \param   left    Left KeyPoint to compare
     * \param   right   RightKeyPoint to compare
     * \return  left._position < right._position
     */
    bool operator< (const TFGeometry::KeyPoint& left, const TFGeometry::KeyPoint& right);

}

#endif // TFGEOMETRY_H__
