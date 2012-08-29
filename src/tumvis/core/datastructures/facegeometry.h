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
// CONTRIBUTORS BE LIABLE FLOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef FACEGEOMETRY_H__
#define FACEGEOMETRY_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
#include "core/datastructures/geometrydata.h"

#include <vector>

namespace TUMVis {

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
     * \note    Like all Geometry classes FaceGeometry has value-sematics: Once created, the 
     *          vertices/colors/etc. cannot be altered anymore.
     */
    class FaceGeometry : public GeometryData {
    public:
        /**
         * Creates a new FaceGeometry.
         * \param   vertices            The list of the vertex positions of the face.
         * \param   textureCoordinates  The list of vertex texture coordinates, may be empty.
         * \param   colors              The list of vertex colors, may be empty.
         * \param   normals             The list of vertex normals, may be empty.
         */
        FaceGeometry(
            const std::vector<tgt::vec3>& vertices,
            const std::vector<tgt::vec3>& textureCoordinates = std::vector<tgt::vec3>(),
            const std::vector<tgt::vec4>& colors = std::vector<tgt::vec4>(),
            const std::vector<tgt::vec3>& normals = std::vector<tgt::vec3>()
            );

        /**
         * Destructor, deletes VBOs/VAO if necessary. Hence, needs a valid OpenGL context
         */
        virtual ~FaceGeometry();

        /// \see AbstractData::clone()
        virtual FaceGeometry* clone() const;

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
         * Renders this FaceGeometry.
         * Must be called from a valid OpenGL context.
         */
        virtual void render();

        /**
         * Creates the OpenGL VBOs and the VAO for this face's geometry.
         * Must be called from a valid OpenGL context.
         */
        virtual void createGLBuffers();


    protected:
        std::vector<tgt::vec3> _vertices;               ///< The list of the vertex positions of the face.
        std::vector<tgt::vec3> _textureCoordinates;     ///< The list of vertex texture coordinates, may be empty.
        std::vector<tgt::vec4> _colors;                 ///< The list of vertex colors, may be empty.
        std::vector<tgt::vec3> _normals;                ///< The list of vertex normals, may be empty.

        tgt::vec3 _faceNormal;                          ///< The normal vector of this face.

        static const std::string loggerCat_;
    };


}

#endif // FACEGEOMETRY_H__
