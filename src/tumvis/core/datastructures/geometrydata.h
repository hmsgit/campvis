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

#include "core/datastructures/abstractdata.h"

namespace tgt {
    class BufferObject;
}

namespace TUMVis {
    /**
     * Abstract base class for data handled by a DataHandle and stored in a DataContainer.
     * 
     * \todo 
     */
    class GeometryData : public AbstractData {
    public:
        GeometryData();

        GeometryData(const GeometryData& rhs);

        /**
         * Destructor, deletes VBOs/VAO if necessary. Hence, needs a valid OpenGL context
         */
        virtual ~GeometryData();

        GeometryData& operator=(const GeometryData& rhs);

        virtual AbstractData* clone() const = 0;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /**
         * Renders this GeometryData.
         * Must be called from a valid OpenGL context.
         */
        virtual void render() const = 0;

        /**
         * Creates the OpenGL VBOs and the VAO for this geometry.
         * Must be called from a valid OpenGL context.
         */
        virtual void createGLBuffers() const = 0;

        const tgt::BufferObject* getVerticesBuffer() const;

        const tgt::BufferObject* getTextureCoordinatesBuffer() const;

        const tgt::BufferObject* getColorsBuffer() const;

        const tgt::BufferObject* getNormalsBuffer() const;

    protected:
        // mutable to support const lazy initialization
        mutable bool _buffersInitialized;
        mutable tgt::BufferObject* _verticesBuffer;
        mutable tgt::BufferObject* _texCoordsBuffer;
        mutable tgt::BufferObject* _colorsBuffer;
        mutable tgt::BufferObject* _normalsBuffer;
    };

}

#endif // GEOMETRYDATA_H__
