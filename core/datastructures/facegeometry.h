// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef FACEGEOMETRY_H__
#define FACEGEOMETRY_H__

#include "cgt/bounds.h"
#include "cgt/vector.h"
#include "core/datastructures/geometrydata.h"

#include <vector>

namespace campvis {

    /**
     * Class for face geometry.
     * Every FaceGeometry has a list of vertices which also defines the number of elements (size). 
     * Colors, normals and texture coordinates are optional. Their lists are either empty or as
     * large as the vertex list. Furthermore, each FaceGeometry has a face normal.
     * 
     * The internal OpenGL buffers are lazy-instantiated.
     * 
     * \note    This class expects all vertices lying within one plane. Everything other that that
     *          leads to undefined behavior.
     */
    class CAMPVIS_CORE_API FaceGeometry : public GeometryData {
    public:
        /**
         * Creates a new empty FaceGeometry.
         */
        explicit FaceGeometry();

        /**
         * Creates a new FaceGeometry initialized from the given data.
         * \param   vertices            The list of the vertex positions of the face.
         * \param   textureCoordinates  The list of vertex texture coordinates, may be empty.
         * \param   colors              The list of vertex colors, may be empty.
         * \param   normals             The list of vertex normals, may be empty.
         */
        explicit FaceGeometry(
            const std::vector<cgt::vec3>& vertices,
            const std::vector<cgt::vec3>& textureCoordinates = std::vector<cgt::vec3>(),
            const std::vector<cgt::vec4>& colors = std::vector<cgt::vec4>(),
            const std::vector<cgt::vec3>& normals = std::vector<cgt::vec3>()
            );

        /**
         * Destructor, deletes VBOs/VAO if necessary.
         */
        virtual ~FaceGeometry();

        /// \see AbstractData::clone()
        virtual FaceGeometry* clone() const;
        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;
        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;

        /**
         * Returns the number of vertices of this face.
         * \return _vertices.size()
         */
        size_t size() const;

        /**
         * The list of the vertex positions of the face.
         * \return _vertices
         */
        const std::vector<cgt::vec3>& getVertices() const;

        /**
         * The list of vertex colors, may be empty.
         * \return _colors
         */
        const std::vector<cgt::vec4>& getColors() const;

        /**
         * The list of vertex normals, may be empty.
         * \return _normals
         */
        const std::vector<cgt::vec3>& getNormals() const;

        /**
         * The list of vertex texture coordinates, may be empty.
         * \return _textureCoordinates
         */
        const std::vector<cgt::vec3>& getTextureCoordinates() const;
        
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

        /**
         * The normal vector of this face.
         * \return _faceNormal
         */
        const cgt::vec3& getFaceNormal() const;

        /**
         * Clips this FaceGeometry against an aribtrary clip plane.
         * The clipped FaceGeometry is returned.
         * \note    Because of using only float internally, the precision is somewhat poor.
         * \param   p       Point on clip plane
         * \param   normal  Clip plane normal
         * \param   epsilon Clipping precision
         * \return  The clipped FaceGeometry
         */
        FaceGeometry clipAgainstPlane(float p, const cgt::vec3& normal, float epsilon = 1e-4f) const;

        /**
         * Renders this FaceGeometry.
         * Must be called from a valid OpenGL context.
         * \param   mode    OpenGL rendering mode for this face
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

        std::vector<cgt::vec3> _vertices;               ///< The list of the vertex positions of the face.
        std::vector<cgt::vec3> _textureCoordinates;     ///< The list of vertex texture coordinates, may be empty.
        std::vector<cgt::vec4> _colors;                 ///< The list of vertex colors, may be empty.
        std::vector<cgt::vec3> _normals;                ///< The list of vertex normals, may be empty.

        std::vector<cgt::col4> _pickingInformation;     ///< The list of picking information colors, max be empty.

        cgt::vec3 _faceNormal;                          ///< The normal vector of this face.

        static const std::string loggerCat_;
    };


}

#endif // FACEGEOMETRY_H__
