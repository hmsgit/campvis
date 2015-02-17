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

#ifndef GEOMETRYFACTORY_H__
#define GEOMETRYFACTORY_H__

#include "cgt/bounds.h"
#include "cgt/vector.h"

#include "core/datastructures/geometrydata.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/meshgeometry.h"
#include "core/datastructures/multiindexedgeometry.h"

#include <vector>

namespace campvis {

    class CAMPVIS_CORE_API GeometryDataFactory {
    public:
        /**
         * Creates a FaceGeometry building an axis-aligned rectangle face in the YX plane (quad) 
         * with the given bounds and texture coordinates.
         * \note    Caller has to take ownership of the returned pointer.
         * \param   llf     Coordinates of the lower left vertex
         * \param   urb     Coordinates of the upper right vertex
         * \param   texLlf  Texture coordinates of the lower left vertex
         * \param   texUrb  Texture coordinates of the upper right vertex
         * \return  A FaceGeometry building a quad with the given bounds and texture coordinates.
         */
        static FaceGeometry* createQuad(const cgt::vec3& llf, const cgt::vec3& urb, const cgt::vec3& texLlf, const cgt::vec3& texUrb);

        /**
         * Creates a MeshGeometry building a cube with the given bounds and texture coordinates.
         * \note    Caller has to take ownership of the returned pointer.
         * \param   bounds      coordinates of the cube bounds
         * \param   texBounds   texture coordinates at the cube bounds
         * \return  A MeshGeometry building a cube with the given bounds and texture coordinates.
         */
        static MeshGeometry* createCube(const cgt::Bounds& bounds, const cgt::Bounds& texBounds);

        /**
         * Creates an MultiIndexedGeometry storing the famous Utah teapot.
         * \return  MultiIndexedGeometry storing the famous Utah teapot.
         */
        static MultiIndexedGeometry* createTeapot();

        /**
         * Creates an MultiIndexedGeometry storing a unit sphere around the origin.
         * \param   numStacks   Number of stacks in the sphere
         * \param   numSlices   Number of slices in the sphere
         * \param   exponents   Exponent for each dimension to define a supersphere (defines the roundness)
         * \return  MultiIndexedGeometry storing a unit sphere around the origin.
         */
        static MultiIndexedGeometry* createSphere(uint16_t numStacks = 6, uint16_t numSlices = 12, const cgt::vec3& exponents = cgt::vec3(1.f));


        /**
         * Creates an MultiIndexedGeometry storing a unit length arrow in Z direction starting from the origin.
         * \param   numSlices	Number of slices in the cylinder and cone
         * \param	tipLen		Length of arrow tip (between 0 and 1)
         * \param   cylRadius	Radius of the cylinder (arrow shaft)
         * \param	tipRadius	Radius of the bottom of the arrow tip
         * \return  MultiIndexedGeometry storing a unit arrow in Z direction starting from the origin.
         */
        static MultiIndexedGeometry* createArrow(uint16_t numSlices = 12, float tipLen = 0.35, float cylRadius = 0.05, float tipRadius = 0.15);
    };

}

#endif // GEOMETRYFACTORY_H__
