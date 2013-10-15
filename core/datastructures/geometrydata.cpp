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

#include "geometrydata.h"
#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"

#include "tgt/qt/qtcontextmanager.h"
#include "core/tools/opengljobprocessor.h"

namespace campvis {

    GeometryData::GeometryData() 
        : AbstractData()
        , _buffersInitialized(false)
        , _verticesBuffer(0)
        , _texCoordsBuffer(0)
        , _colorsBuffer(0)
        , _normalsBuffer(0)
        , _context(0)
    {
    }

    GeometryData::GeometryData(const GeometryData& rhs)
        : AbstractData(rhs)
        , _buffersInitialized(false)
        , _verticesBuffer(0)
        , _texCoordsBuffer(0)
        , _colorsBuffer(0)
        , _normalsBuffer(0)
    {

    }

    GeometryData::~GeometryData() {
        if (_buffersInitialized) {
            delete _verticesBuffer;
            delete _texCoordsBuffer;
            delete _colorsBuffer;
            delete _normalsBuffer;
//             std::vector<tgt::BufferObject*> buffers(_buffers, _buffers + NUM_BUFFERS);
//             GLJobProc.enqueueJob(_context, makeJobOnHeap(&GeometryData::deleteBuffers, buffers), OpenGLJobProcessor::LowPriorityJob);
        }
    }

    GeometryData& GeometryData::operator=(const GeometryData& rhs) {
        if (this == &rhs)
            return *this;

        AbstractData::operator=(rhs);

        // delete old VBOs and null pointers
        if (_buffersInitialized) {
            delete _verticesBuffer;
            delete _texCoordsBuffer;
            delete _colorsBuffer;
            delete _normalsBuffer;
//             std::vector<tgt::BufferObject*> buffers(_buffers, _buffers + NUM_BUFFERS);
//             GLJobProc.enqueueJob(_context, makeJobOnHeap(&GeometryData::deleteBuffers, buffers), OpenGLJobProcessor::LowPriorityJob);
        }

        _verticesBuffer = 0;
        _texCoordsBuffer = 0;
        _colorsBuffer = 0;
        _normalsBuffer = 0;
        _buffersInitialized = false;

        return *this;
    }

    void GeometryData::deleteBuffers(std::vector<tgt::BufferObject*> buffers) {
        for (std::vector<tgt::BufferObject*>::iterator it = buffers.begin(); it != buffers.end(); ++it)
            delete *it;
    }

    size_t GeometryData::getVideoMemoryFootprint() const {
        size_t sum = 0;

        if (_verticesBuffer != 0)
            sum += _verticesBuffer->getBufferSize();
        if (_texCoordsBuffer != 0)
            sum += _texCoordsBuffer->getBufferSize();
        if (_colorsBuffer != 0)
            sum += _colorsBuffer->getBufferSize();
        if (_normalsBuffer != 0)
            sum += _normalsBuffer->getBufferSize();

        return sum;
    }

    void GeometryData::createGLBuffers() const {
        if (! _buffersInitialized) 
            _context = CtxtMgr.getCurrentContext();
    }

    const tgt::BufferObject* GeometryData::getVerticesBuffer() const {
        return _verticesBuffer;
    }

    const tgt::BufferObject* GeometryData::getTextureCoordinatesBuffer() const {
        return _texCoordsBuffer;
    }

    const tgt::BufferObject* GeometryData::getColorsBuffer() const {
        return _colorsBuffer;
    }

    const tgt::BufferObject* GeometryData::getNormalsBuffer() const {
        return _normalsBuffer;
    }

}