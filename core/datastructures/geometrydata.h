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

#ifndef GEOMETRYDATA_H__
#define GEOMETRYDATA_H__

#include "tgt/tgt_gl.h"
#include "tgt/bounds.h"
#include "core/datastructures/abstractdata.h"
#include <vector>

namespace tgt {
    class BufferObject;
    class GLCanvas;
}

namespace campvis {
    /**
     * Abstract base class for geometry data in CAMPVis.
     * 
     * GeometryData consists of a set of vertices (vec3) and optionally colors (vec4), texture 
     * coordinates (vec3) and/or normals (vec3) for each vertex.
     * GeometryData stores the geometry on the CPU side (local RAM) and takes care of transparently 
     * mapping it into GPU memory in order to render it using OpenGL. The mapping of vertex 
     * information to OpenGL vertex attributes is as follows:
     *  - Vertex positions: Vertex attribute 0
     *  - Vertex texture coordinates: Vertex attribute 1
     *  - Vertex colors: Vertex attribute 2
     *  - Vertex normals: Vertex attribute 3
     * 
     */    
    class GeometryData : public AbstractData, public IHasWorldBounds {
    public:
        /**
         * Constructor
         * \param   vertexFeatures  List of features present for each vertex
         */
        explicit GeometryData();

        /**
         * Copy constructor
         * \param   rhs GeometryData to copy
         */
        GeometryData(const GeometryData& rhs);

        /**
         * Destructor, deletes VBOs/VAO if necessary.
         */
        virtual ~GeometryData();

        /**
         * Assignment operator.
         * \param   rhs GeometryData to assign to this.
         * \return  *this after assignment
         */
        GeometryData& operator=(const GeometryData& rhs);

        /// \see AbstractData::clone()
        virtual AbstractData* clone() const = 0;

        /**
         * Renders this GeometryData.
         * Must be called from a valid OpenGL context.
         */
        virtual void render(GLenum mode = GL_POLYGON) const = 0;
        
        /**
         * Returns the geometry extent in world coordinates.
         * \return  The geometry extent in world coordinates.
         */
        virtual tgt::Bounds getWorldBounds() const = 0;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex positions.
         * May be 0 if not yet created.
         * \return  _verticesBuffer
         */
        const tgt::BufferObject* getVerticesBuffer() const;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex texture coordinates.
         * May be 0 if none are present or not yet created.
         * \return  _texCoordsBuffer
         */
        const tgt::BufferObject* getTextureCoordinatesBuffer() const;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex colors. 
         * May be 0 if none are present or not yet created.
         * \return  _colorsBuffer
         */
        const tgt::BufferObject* getColorsBuffer() const;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex normals.
         * May be 0 if none are present or not yet created.
         * \return  _normalsBuffer
         */
        const tgt::BufferObject* getNormalsBuffer() const;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

    protected:
        /**
         * Deletes all OpenGL BufferObjects.
         */
        void deleteBuffers() const;

        // mutable to support const lazy initialization
        mutable bool _buffersDirty;             ///< Flag whether the buffers are dirty (i.e. need to be (re)initialized)

        enum { NUM_BUFFERS = 4 };               ///< Number of buffers in _buffers array

        union {
            struct {
                mutable tgt::BufferObject* _verticesBuffer;     ///< Pointer to the OpenGL Buffer with the vertex positions
                mutable tgt::BufferObject* _texCoordsBuffer;    ///< Pointer to the OpenGL Buffer with the vertex texture coordinates
                mutable tgt::BufferObject* _colorsBuffer;       ///< Pointer to the OpenGL Buffer with the vertex colors
                mutable tgt::BufferObject* _normalsBuffer;      ///< Pointer to the OpenGL Buffer with the vertex normals
            };

            mutable tgt::BufferObject* _buffers[NUM_BUFFERS];   ///< Array of all buffers
        };

    private:

        static const std::string loggerCat_;
    };

}

#endif // GEOMETRYDATA_H__
