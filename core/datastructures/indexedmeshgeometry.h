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

#ifndef INDEXEDIndexedMeshGeometry_H__
#define INDEXEDIndexedMeshGeometry_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
#include "core/datastructures/geometrydata.h"
#include "core/datastructures/facegeometry.h"

#include <vector>

namespace campvis {

    /**
     * Class for mesh geometry.
     * Every IndexedMeshGeometry consists of a stream of vertices and an index list defining the faces.
     * 
     * The internal OpenGL buffers are lazy-instantiated.
     * 
     * \note    Like all Geometry classes IndexedMeshGeometry has value-sematics: Once created, the 
     *          faces cannot be altered anymore.
     */
    class CAMPVIS_CORE_API IndexedMeshGeometry : public GeometryData {
    public:
        /**
         * Creates a new IndexedMeshGeometry built from the given faces.
         * \param   indices             Index list defining the faces.
         * \param   vertices            The list of the vertex positions of the face.
         * \param   textureCoordinates  The list of vertex texture coordinates, may be empty.
         * \param   colors              The list of vertex colors, may be empty.
         * \param   normals             The list of vertex normals, may be empty.
         */
        explicit IndexedMeshGeometry(
            const std::vector<uint16_t>& indices,
            const std::vector<tgt::vec3>& vertices,
            const std::vector<tgt::vec3>& textureCoordinates = std::vector<tgt::vec3>(),
            const std::vector<tgt::vec4>& colors = std::vector<tgt::vec4>(),
            const std::vector<tgt::vec3>& normals = std::vector<tgt::vec3>()
            );
        
        /**
         * Copy constructor
         * \param   rhs IndexedMeshGeometry to copy
         */
        IndexedMeshGeometry(const IndexedMeshGeometry& rhs);

        /**
         * Destructor, deletes VBOs/VAO if necessary. Hence, needs a valid OpenGL context
         */
        virtual ~IndexedMeshGeometry();
        
        /**
         * Assignment operator.
         * \param   rhs IndexedMeshGeometry to assign to this.
         * \return  *this after assignment
         */
        IndexedMeshGeometry& operator=(const IndexedMeshGeometry& rhs);

        /// \see AbstractData::clone()
        virtual IndexedMeshGeometry* clone() const;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /**
         * Renders this IndexedMeshGeometry.
         * Must be called from a valid OpenGL context.
         * \param   mode    OpenGL rendering mode for this mesh
         */
        virtual void render(GLenum mode) const;

        /// \see GeometryData::getWorldBounds
        virtual tgt::Bounds getWorldBounds() const;

    protected:
        /**
         * Creates the OpenGL VBOs and the VAO for this face's geometry.
         * Must be called from a valid OpenGL context.
         */
        void createGLBuffers() const;

        /// Deletes the OpenGL BufferObject for the indices.
        void deleteIndicesBuffer() const;

        std::vector<uint16_t> _indices;                 ///< Index list defining the faces
        std::vector<tgt::vec3> _vertices;               ///< The list of the vertex positions of the face.
        std::vector<tgt::vec3> _textureCoordinates;     ///< The list of vertex texture coordinates, may be empty.
        std::vector<tgt::vec4> _colors;                 ///< The list of vertex colors, may be empty.
        std::vector<tgt::vec3> _normals;                ///< The list of vertex normals, may be empty.

        mutable tgt::BufferObject* _indicesBuffer;

        static const std::string loggerCat_;
    };

}

#endif // INDEXEDIndexedMeshGeometry_H__
