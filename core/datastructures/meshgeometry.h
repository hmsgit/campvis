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

#ifndef MESHGEOMETRY_H__
#define MESHGEOMETRY_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
#include "core/datastructures/geometrydata.h"
#include "core/datastructures/facegeometry.h"

#include <vector>

namespace campvis {

    /**
     * Class for mesh geometry.
     * Every MeshGeometry consists of a list of FaceGeometry objects.
     * 
     * The internal OpenGL buffers are lazy-instantiated.
     */
    class CAMPVIS_CORE_API MeshGeometry : public GeometryData {
    public:
        /**
         * Creates a new MeshGeometry built from the given faces.
         */
        explicit MeshGeometry(const std::vector<FaceGeometry>& faces);

        /**
         * Destructor, deletes VBOs/VAO if necessary. Hence, needs a valid OpenGL context
         */
        virtual ~MeshGeometry();

        /// \see AbstractData::clone()
        virtual MeshGeometry* clone() const;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /**
         * Returns the number of faces of this mesh.
         * \return _faces.size()
         */
        size_t size() const;

        /**
         * Gets the list of all faces of this mesh.
         * \return  _faces
         */
        const std::vector<FaceGeometry>& getFaces() const;

        /**
         * Clips this MeshGeometry against an aribtrary clip plane.
         * The clipped MeshGeometry is returned.
         * \note    When clipping against one of the faces of this mesh, the mesh closing 
         *          algorithm might fail and return an invalid mesh!
         * \param   p       Point on clip plane
         * \param   normal  Clip plane normal
         * \param   close   Flag, whether the returned mesh shall be closed. 
         *                  If true, the closing face will be the last one in the returned MeshGeometry.
         * \param   epsilon Clipping precision
         * \return  The clipped MeshGeometry
         */
        MeshGeometry clipAgainstPlane(float p, const tgt::vec3& normal, bool close = true, float epsilon = 1e-4f) const;

        /**
         * Renders this MeshGeometry.
         * Must be called from a valid OpenGL context.
         * \param   mode    OpenGL rendering mode for this mesh
         */
        virtual void render(GLenum mode) const;

        /// \see GeometryData::getWorldBounds
        virtual tgt::Bounds getWorldBounds() const;
        /// \see GeometryData::hasTextureCoordinates
        virtual bool hasTextureCoordinates() const;
        /// \see GeometryData::applyTransformationToVertices
        virtual void applyTransformationToVertices(const tgt::mat4& t);

    protected:
        /**
         * Creates the OpenGL VBOs and the VAO for this face's geometry.
         * Must be called from a valid OpenGL context.
         */
        void createGLBuffers() const;

        std::vector<FaceGeometry> _faces;           ///< List of all faces of this mesh

        static const std::string loggerCat_;
    };

}

#endif // MESHGEOMETRY_H__
