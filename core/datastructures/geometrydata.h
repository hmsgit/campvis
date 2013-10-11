// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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
     * Abstract base class for data handled by a DataHandle and stored in a DataContainer.
     * 
     * \todo 
     */
    class GeometryData : public AbstractData, public IHasWorldBounds {
    public:
        GeometryData();

        GeometryData(const GeometryData& rhs);

        /**
         * Destructor, deletes VBOs/VAO if necessary. Hence, needs a valid OpenGL context
         */
        virtual ~GeometryData();

        GeometryData& operator=(const GeometryData& rhs);

        static void deleteBuffers(std::vector<tgt::BufferObject*> buffers);

        virtual AbstractData* clone() const = 0;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

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
         * Creates the OpenGL VBOs and the VAO for this geometry.
         * Must be called from a valid OpenGL context.
         * \note    When overwriting this method, make sure to call base class method first!
         */
        virtual void createGLBuffers() const;

        const tgt::BufferObject* getVerticesBuffer() const;

        const tgt::BufferObject* getTextureCoordinatesBuffer() const;

        const tgt::BufferObject* getColorsBuffer() const;

        const tgt::BufferObject* getNormalsBuffer() const;

    protected:
        // mutable to support const lazy initialization
        mutable bool _buffersInitialized;

        enum { NUM_BUFFERS = 4 };

        union {
            struct {
                mutable tgt::BufferObject* _verticesBuffer;
                mutable tgt::BufferObject* _texCoordsBuffer;
                mutable tgt::BufferObject* _colorsBuffer;
                mutable tgt::BufferObject* _normalsBuffer;
            };

            mutable tgt::BufferObject* _buffers[NUM_BUFFERS];
        };

    private:
        mutable tgt::GLCanvas* _context;        ///< OpenGL context the buffers were created in (so that they can be deleted correctly)
    };

}

#endif // GEOMETRYDATA_H__
