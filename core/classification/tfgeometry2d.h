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

#ifndef TFGEOMETRY2D_H__
#define TFGEOMETRY2D_H__

#include "sigslot/sigslot.h"
#include "tgt/vector.h"
#include <vector>

#include "core/coreapi.h"

namespace tgt {
    class Texture;
}

namespace campvis {

    /**
     * Defines a single shape for the GeometryTransferFunction class.
     * TFGeometry2D is defined by a sorted list of KeyPoints, each having a position and a color.
     */
    class CAMPVIS_CORE_API TFGeometry2D {
    public:
        struct CAMPVIS_CORE_API KeyPoint {
            explicit KeyPoint(const tgt::vec2& position, const tgt::col4& color)
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
        explicit TFGeometry2D(const std::vector<KeyPoint>& keyPoints);

        /**
         * Virtual destructor
         */
        virtual ~TFGeometry2D();
        
        /**
         * Clones this transfer function geometry.
         * \return  A clone of this transfer function geometry, caller takes ownership of the pointer.
         */
        TFGeometry2D* clone() const;

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
        sigslot::signal0 s_changed;

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
