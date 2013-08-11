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

#include "fiberdata.h"

#include "tgt/buffer.h"
#include "tgt/logmanager.h"
#include "tgt/vertexarrayobject.h"

namespace campvis {

    FiberData::FiberData() 
        : AbstractData()
        , _vertexBuffer(0)
        , _tangentBuffer(0)
        , _buffersInitialized(false)
        , _vboFiberStartIndices(0)
        , _vboFiberCounts(0)
    {
    }

    FiberData::FiberData(const FiberData& rhs)
        : AbstractData(rhs)
        , _vertices(rhs._vertices)
        , _fibers(rhs._fibers)
        , _vertexBuffer(0)
        , _tangentBuffer(0)
        , _buffersInitialized(false)
        , _vboFiberStartIndices(0)
        , _vboFiberCounts(0)
    {}

    FiberData::~FiberData() {
        delete _vertexBuffer;
        delete _tangentBuffer;
        delete [] _vboFiberStartIndices;
        delete [] _vboFiberCounts;
    }

    FiberData& FiberData::operator=(const FiberData& rhs) {
        if (this == &rhs)
            return *this;

        AbstractData::operator=(rhs);

        _vertices = rhs._vertices;
        _fibers = rhs._fibers;

        // delete old VBOs and null pointers
        delete _vertexBuffer;
        delete _tangentBuffer;

        _vertexBuffer = 0;
        _tangentBuffer = 0;
        _buffersInitialized = false;

        return *this;

    }

    void FiberData::addFiber(const std::deque<tgt::vec3>& vertices) {
        _vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
        _fibers.push_back(Fiber(_vertices.size() - vertices.size(), _vertices.size()));
        _buffersInitialized = false;
    }

    void FiberData::addFiber(const std::vector<tgt::vec3>& vertices) {
        _vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
        _fibers.push_back(Fiber(_vertices.size() - vertices.size(), _vertices.size()));
        _buffersInitialized = false;
    }

    void FiberData::clear() {
        _fibers.clear();
        _vertices.clear();
        _buffersInitialized = false;
    }

    void FiberData::updateLengths() const {
        for (size_t i = 0; i < _fibers.size(); ++i) {
            _fibers[i]._length = 0.0f;

            for (size_t j = _fibers[i]._startIndex + 1; j < _fibers[i]._endIndex; ++j)
                _fibers[i]._length += distance(_vertices[j-1], _vertices[j]);
        }
    }

    size_t FiberData::numFibers() const {
        return _fibers.size();
    }

    size_t FiberData::numSegments() const {
        size_t sum = 0;
        for (std::vector<Fiber>::const_iterator it = _fibers.begin(); it != _fibers.end(); ++it) 
            sum += (it->_endIndex - it->_startIndex);
        return sum;
    }


    bool FiberData::empty() const {
        return _fibers.empty();
    }

    FiberData* FiberData::clone() const {
        FiberData* toReturn = new FiberData(*this);
        return toReturn;
    }

    size_t FiberData::getLocalMemoryFootprint() const {
        size_t sum = _vertices.size() * sizeof(tgt::vec3);
        sum += _fibers.size() * sizeof(Fiber);
        sum += sizeof(*this);
        return sum;
    }

    size_t FiberData::getVideoMemoryFootprint() const {
        size_t sum = 0;
        if (_vertexBuffer != 0)
            sum += _vertexBuffer->getBufferSize();
        if (_tangentBuffer != 0)
            sum += _tangentBuffer->getBufferSize();

        return sum;
    }

    void FiberData::createGlBuffers() const {
        if (_buffersInitialized)
            return;

        // reset everything
        delete _vertexBuffer;
        delete _tangentBuffer;
        delete [] _vboFiberStartIndices;
        delete [] _vboFiberCounts;
        _vboFiberArraySize = 0;
        _vboFiberStartIndices = new GLint[_vertices.size()];
        _vboFiberCounts = new GLint[_vertices.size()];

        std::vector<tgt::vec3> tangents;
        tangents.resize(_vertices.size());

        for (std::vector<Fiber>::const_iterator it = _fibers.begin(); it != _fibers.end(); ++it) {
            _vboFiberStartIndices[_vboFiberArraySize] = static_cast<GLint>(it->_startIndex);
            _vboFiberCounts[_vboFiberArraySize] = static_cast<GLsizei>(it->_endIndex - it->_startIndex);
            ++_vboFiberArraySize;

            tgt::vec3 dirPrev = tgt::vec3::zero;
            tgt::vec3 dirNext = tgt::vec3::zero;

            for (size_t i = it->_startIndex; i < it->_endIndex-1; ++i) {
                dirNext = _vertices[i+1] - _vertices[i];
                tangents[i] = tgt::normalize(dirPrev + dirNext);
                dirPrev = dirNext;
            }

            tangents[it->_endIndex - 1] = dirPrev;
        }

        try {
            _vertexBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
            _vertexBuffer->data(&_vertices.front(), _vertices.size() * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);

            _tangentBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
            _tangentBuffer->data(&tangents.front(), tangents.size() * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);
        }
        catch (tgt::Exception& e) {
            LERRORC("CAMPVis.modules.columbia.FiberData", "Error creating OpenGL Buffer objects: " << e.what());
            _buffersInitialized = false;
            return;
        }

        LGL_ERROR;
        _buffersInitialized = true;
    }

    void FiberData::render(GLenum mode /*= GL_LINE_STRIP*/) const {
        createGlBuffers();
        if (! _buffersInitialized) {
            LERRORC("CAMPVis.modules.columbia.FiberData", "Cannot render without initialized OpenGL buffers.");
            return;
        }

        tgt::VertexArrayObject vao;
        vao.addVertexAttribute(tgt::VertexArrayObject::VerticesAttribute, _vertexBuffer);
        vao.addVertexAttribute(tgt::VertexArrayObject::NormalsAttribute, _tangentBuffer);
        LGL_ERROR;

        glMultiDrawArrays(mode, _vboFiberStartIndices, _vboFiberCounts, _vboFiberArraySize);
        LGL_ERROR;

    }

    tgt::Bounds FiberData::getWorldBounds() const {
        tgt::Bounds toReturn;
        for (std::vector<tgt::vec3>::const_iterator it = _vertices.begin(); it != _vertices.end(); ++it)
            toReturn.addPoint(*it);
        return toReturn;
    }

}