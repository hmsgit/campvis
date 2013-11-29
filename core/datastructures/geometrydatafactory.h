// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef GEOMETRYFACTORY_H__
#define GEOMETRYFACTORY_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
#include "core/datastructures/geometrydata.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/meshgeometry.h"

#include <vector>

namespace campvis {

    class GeometryDataFactory {
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
        static FaceGeometry* createQuad(const tgt::vec3& llf, const tgt::vec3& urb, const tgt::vec3& texLlf, const tgt::vec3& texUrb);

        /**
         * Creates a MeshGeometry building a cube with the given bounds and texture coordinates.
         * \note    Caller has to take ownership of the returned pointer.
         * \param   bounds      coordinates of the cube bounds
         * \param   texBounds   texture coordinates at the cube bounds
         * \return  A MeshGeometry building a cube with the given bounds and texture coordinates.
         */
        static MeshGeometry* createCube(const tgt::Bounds& bounds, const tgt::Bounds& texBounds);

    };

}

#endif // GEOMETRYFACTORY_H__
