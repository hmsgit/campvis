// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef TFGEOMETRY1D_H__
#define TFGEOMETRY1D_H__

#include "sigslot/sigslot.h"
#include "tgt/vector.h"
#include <vector>

namespace tgt {
    class Texture;
}

namespace campvis {
    class FaceGeometry;

    /**
     * Defines a single shape for the GeometryTransferFunction class.
     * TFGeometry1D is defined by a sorted list of KeyPoints, each having a position and a color.
     */
    class TFGeometry1D {
    public:
        struct KeyPoint {
            explicit KeyPoint(float position, const tgt::col4& color)
                : _position(position)
                , _color(color)
            {};

            float _position;
            tgt::col4 _color;
        };

        /**
         * Creates a new TFGeometry1D
         * \param   Bounds  Bounds of the position of the geometry in texture coordinates.
         */
        explicit TFGeometry1D(const std::vector<KeyPoint>& keyPoints);

        /**
         * Virtual destructor
         */
        virtual ~TFGeometry1D();

        /**
         * Clones this transfer function geometry.
         * \return  A clone of this transfer function geometry, caller takes ownership of the pointer.
         */
        TFGeometry1D* clone() const;

        /**
         * Returns the vector of KeyPoints.
         * \return 
         */
        std::vector<KeyPoint>& getKeyPoints();
        
        /**
         * Renders this transfer function geometry to the current active OpenGL context for usage in editor.
         * \note    Must be called from an active and valid OpenGL context.
         * \todo    Check, whether this method really belongs here (core) or better fits into
         *          an other class in the application (GUI) module.
         */
        void renderIntoEditor() const;

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
         * \param   interval    Interval the geometry resides in
         * \param   leftColor   Color for left KeyPoint
         * \param   rightColor  Color for right KeyPoint
         * \return  A TFGeometry1D modelling a quad with two KeyPoints.
         */
        static TFGeometry1D* createQuad(const tgt::vec2& interval, const tgt::col4& leftColor, const tgt::vec4& rightColor);
    protected:

        std::vector<KeyPoint> _keyPoints;       ///< vector of KeyPoints, KeyPoints are sorted by x-coordinate of the position
        FaceGeometry* _tfRenderFace;            ///< FaceGeometry used to render TF into the TF texture
        FaceGeometry* _tfEditorFace;            ///< FaceGeometry used to render TF into editor window
    };

// ================================================================================================

    /**
     * Less operator for sorting KeyPoints by their position.
     * \param   left    Left KeyPoint to compare
     * \param   right   RightKeyPoint to compare
     * \return  left._position < right._position
     */
    bool operator< (const TFGeometry1D::KeyPoint& left, const TFGeometry1D::KeyPoint& right);

}

#endif // TFGEOMETRY1D_H__
