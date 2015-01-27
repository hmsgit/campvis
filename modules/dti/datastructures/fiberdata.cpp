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

#include "fiberdata.h"

#include "cgt/buffer.h"
#include "cgt/logmanager.h"
#include "cgt/vertexarrayobject.h"

namespace campvis {
namespace dti {

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

    void FiberData::addFiber(const std::deque<cgt::vec3>& vertices) {
        _vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
        _fibers.push_back(Fiber(_vertices.size() - vertices.size(), _vertices.size()));
        _buffersInitialized = false;
    }

    void FiberData::addFiber(const std::vector<cgt::vec3>& vertices) {
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
        size_t sum = _vertices.size() * sizeof(cgt::vec3);
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
        if (_buffersInitialized || _fibers.empty() || _vertices.empty())
            return;

        // reset everything
        delete _vertexBuffer;
        delete _tangentBuffer;
        delete [] _vboFiberStartIndices;
        delete [] _vboFiberCounts;
        _vboFiberArraySize = 0;
        _vboFiberStartIndices = new GLint[_vertices.size()];
        _vboFiberCounts = new GLint[_vertices.size()];

        std::vector<cgt::vec3> tangents;
        tangents.resize(_vertices.size());

        for (std::vector<Fiber>::const_iterator it = _fibers.begin(); it != _fibers.end(); ++it) {
            if (it->_visible) {
                _vboFiberStartIndices[_vboFiberArraySize] = static_cast<GLint>(it->_startIndex);
                _vboFiberCounts[_vboFiberArraySize] = static_cast<GLsizei>(it->_endIndex - it->_startIndex);
                ++_vboFiberArraySize;
            }

            cgt::vec3 dirPrev = cgt::vec3::zero;
            cgt::vec3 dirNext = cgt::vec3::zero;

            for (size_t i = it->_startIndex; i < it->_endIndex-1; ++i) {
                dirNext = _vertices[i+1] - _vertices[i];
                tangents[i] = cgt::normalize(dirPrev + dirNext);
                dirPrev = dirNext;
            }

            tangents[it->_endIndex - 1] = dirPrev;
        }

        try {
            _vertexBuffer = new cgt::BufferObject(cgt::BufferObject::ARRAY_BUFFER, cgt::BufferObject::USAGE_STATIC_DRAW);
            _vertexBuffer->data(&_vertices.front(), _vertices.size() * sizeof(cgt::vec3), cgt::BufferObject::FLOAT, 3);

            _tangentBuffer = new cgt::BufferObject(cgt::BufferObject::ARRAY_BUFFER, cgt::BufferObject::USAGE_STATIC_DRAW);
            _tangentBuffer->data(&tangents.front(), tangents.size() * sizeof(cgt::vec3), cgt::BufferObject::FLOAT, 3);
        }
        catch (cgt::Exception& e) {
            LERRORC("CAMPVis.modules.columbia.FiberData", "Error creating OpenGL Buffer objects: " << e.what());
            _buffersInitialized = false;
            return;
        }

        LGL_ERROR;
        _buffersInitialized = true;
    }

    void FiberData::render(GLenum mode /*= GL_LINE_STRIP*/) const {
        if (_fibers.empty() || _vertices.empty())
            return;

        createGlBuffers();
        if (! _buffersInitialized) {
            LERRORC("CAMPVis.modules.columbia.FiberData", "Cannot render without initialized OpenGL buffers.");
            return;
        }

        cgt::VertexArrayObject vao;
        vao.setVertexAttributePointer(0, _vertexBuffer);
        vao.setVertexAttributePointer(1, _tangentBuffer);
        LGL_ERROR;

        glMultiDrawArrays(mode, _vboFiberStartIndices, _vboFiberCounts, _vboFiberArraySize);
        LGL_ERROR;

    }

    cgt::Bounds FiberData::getWorldBounds() const {
        cgt::Bounds toReturn;
        for (std::vector<cgt::vec3>::const_iterator it = _vertices.begin(); it != _vertices.end(); ++it)
            toReturn.addPoint(*it);
        return toReturn;
    }

    void FiberData::setVisible(size_t index, bool visibility) {
        _fibers[index]._visible = visibility;
        _buffersInitialized = false;
    }

    const std::vector<FiberData::Fiber>& FiberData::getFibers() const {
        return _fibers;
    }

    const std::vector<cgt::vec3>& FiberData::getVertices() const {
        return _vertices;
    }

    std::string FiberData::getTypeAsString() const {
        return "FiberData";
    }

}
}
