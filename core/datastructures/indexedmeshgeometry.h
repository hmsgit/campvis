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
    class IndexedMeshGeometry : public GeometryData {
    public:
        /**
         * Creates a new IndexedMeshGeometry built from the given faces.
         * \param   indices             Index list defining the faces.
         * \param   vertices            The list of the vertex positions of the face.
         * \param   textureCoordinates  The list of vertex texture coordinates, may be empty.
         * \param   colors              The list of vertex colors, may be empty.
         * \param   normals             The list of vertex normals, may be empty.
         */
        IndexedMeshGeometry(
            const std::vector<uint16_t>& indices,
            const std::vector<tgt::vec3>& vertices,
            const std::vector<tgt::vec3>& textureCoordinates = std::vector<tgt::vec3>(),
            const std::vector<tgt::vec4>& colors = std::vector<tgt::vec4>(),
            const std::vector<tgt::vec3>& normals = std::vector<tgt::vec3>()
            );

        /**
         * Destructor, deletes VBOs/VAO if necessary. Hence, needs a valid OpenGL context
         */
        virtual ~IndexedMeshGeometry();

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

        /**
         * Creates the OpenGL VBOs and the VAO for this face's geometry.
         * Must be called from a valid OpenGL context.
         */
        virtual void createGLBuffers() const;

        /// \see GeometryData::getWorldBounds
        virtual tgt::Bounds getWorldBounds() const;

    protected:

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
