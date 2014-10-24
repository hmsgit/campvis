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

#ifndef INDEXEDMESHGEOMETRY_H__
#define INDEXEDMESHGEOMETRY_H__

#include "cgt/bounds.h"
#include "cgt/vector.h"
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
            const std::vector<cgt::vec3>& vertices,
            const std::vector<cgt::vec3>& textureCoordinates = std::vector<cgt::vec3>(),
            const std::vector<cgt::vec4>& colors = std::vector<cgt::vec4>(),
            const std::vector<cgt::vec3>& normals = std::vector<cgt::vec3>()
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


        /**
         * The list of picking information colors, may be empty.
         * \return  _pickingInformation
         */
        const std::vector<cgt::col4>& getPickingInformation() const;

        /**
         * Sets the picking information of this geometry to \a pickingInformation
         * \param   pickingInformation  The new list of picking information for this geometry
         */
        void setPickingInformation(const std::vector<cgt::col4>& pickingInformation);


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
        virtual cgt::Bounds getWorldBounds() const;
        /// \see GeometryData::hasTextureCoordinates
        virtual bool hasTextureCoordinates() const;
        /// \see GeometryData::hasPickingInformation
        virtual bool hasPickingInformation() const;
        /// \see GeometryData::applyTransformationToVertices
        virtual void applyTransformationToVertices(const cgt::mat4& t);

    protected:
        /**
         * Creates the OpenGL VBOs and the VAO for this face's geometry.
         * Must be called from a valid OpenGL context.
         */
        void createGLBuffers() const;

        /// Deletes the OpenGL BufferObject for the indices.
        void deleteIndicesBuffer() const;

        std::vector<uint16_t> _indices;                 ///< Index list defining the faces
        std::vector<cgt::vec3> _vertices;               ///< The list of the vertex positions of the face.
        std::vector<cgt::vec3> _textureCoordinates;     ///< The list of vertex texture coordinates, may be empty.
        std::vector<cgt::vec4> _colors;                 ///< The list of vertex colors, may be empty.
        std::vector<cgt::vec3> _normals;                ///< The list of vertex normals, may be empty.

        std::vector<cgt::col4> _pickingInformation;     ///< The list of picking information colors, max be empty.

        mutable cgt::BufferObject* _indicesBuffer;

        static const std::string loggerCat_;
    };

}

#endif // INDEXEDMESHGEOMETRY_H__
