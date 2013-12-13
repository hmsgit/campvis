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

#ifndef FACEGEOMETRY_H__
#define FACEGEOMETRY_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
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
    class FaceGeometry : public GeometryData {
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
            const std::vector<tgt::vec3>& vertices,
            const std::vector<tgt::vec3>& textureCoordinates = std::vector<tgt::vec3>(),
            const std::vector<tgt::vec4>& colors = std::vector<tgt::vec4>(),
            const std::vector<tgt::vec3>& normals = std::vector<tgt::vec3>()
            );

        /**
         * Destructor, deletes VBOs/VAO if necessary.
         */
        virtual ~FaceGeometry();

        /// \see AbstractData::clone()
        virtual FaceGeometry* clone() const;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /**
         * Returns the number of vertices of this face.
         * \return _vertices.size()
         */
        size_t size() const;

        /**
         * The list of the vertex positions of the face.
         * \return _vertices
         */
        const std::vector<tgt::vec3>& getVertices() const;

        /**
         * The list of vertex colors, may be empty.
         * \return _colors
         */
        const std::vector<tgt::vec4>& getColors() const;

        /**
         * The list of vertex normals, may be empty.
         * \return _normals
         */
        const std::vector<tgt::vec3>& getNormals() const;

        /**
         * The list of vertex texture coordinates, may be empty.
         * \return _textureCoordinates
         */
        const std::vector<tgt::vec3>& getTextureCoordinates() const;

        /**
         * The normal vector of this face.
         * \return _faceNormal
         */
        const tgt::vec3& getFaceNormal() const;

        /**
         * Clips this FaceGeometry against an aribtrary clip plane.
         * The clipped FaceGeometry is returned.
         * \note    Because of using only float internally, the precision is somewhat poor.
         * \param   p       Point on clip plane
         * \param   normal  Clip plane normal
         * \param   epsilon Clipping precision
         * \return  The clipped FaceGeometry
         */
        FaceGeometry clipAgainstPlane(float p, const tgt::vec3& normal, float epsilon = 1e-4f) const;

        /**
         * Renders this FaceGeometry.
         * Must be called from a valid OpenGL context.
         * \param   mode    OpenGL rendering mode for this face
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

        std::vector<tgt::vec3> _vertices;               ///< The list of the vertex positions of the face.
        std::vector<tgt::vec3> _textureCoordinates;     ///< The list of vertex texture coordinates, may be empty.
        std::vector<tgt::vec4> _colors;                 ///< The list of vertex colors, may be empty.
        std::vector<tgt::vec3> _normals;                ///< The list of vertex normals, may be empty.

        tgt::vec3 _faceNormal;                          ///< The normal vector of this face.

        static const std::string loggerCat_;
    };


}

#endif // FACEGEOMETRY_H__
