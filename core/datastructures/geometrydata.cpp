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

#include "geometrydata.h"
#include "tgt/buffer.h"
#include "tgt/logmanager.h"
#include "tgt/glcontextmanager.h"
#include "tgt/vertexarrayobject.h"

#include "core/tools/opengljobprocessor.h"

namespace campvis {

    const std::string GeometryData::loggerCat_ = "CAMPVis.core.datastructures.GeometryData";;

    GeometryData::GeometryData() 
        : AbstractData()
        , _buffersDirty(true)
        , _verticesBuffer(nullptr)
        , _texCoordsBuffer(nullptr)
        , _colorsBuffer(nullptr)
        , _normalsBuffer(nullptr)
        , _pickingBuffer(nullptr)
    {
    }

    GeometryData::GeometryData(const GeometryData& rhs)
        : AbstractData(rhs)
        , _buffersDirty(true)
        , _verticesBuffer(nullptr)
        , _texCoordsBuffer(nullptr)
        , _colorsBuffer(nullptr)
        , _normalsBuffer(nullptr)
        , _pickingBuffer(nullptr)
    {

    }

    GeometryData::~GeometryData() {
        deleteBuffers();
    }

    GeometryData& GeometryData::operator=(const GeometryData& rhs) {
        if (this == &rhs)
            return *this;

        AbstractData::operator=(rhs);

        // delete old VBOs and null pointers
        deleteBuffers();

        return *this;
    }

    void GeometryData::deleteBuffers() const {
        for (int i = 0; i < NUM_BUFFERS; ++i) {
            delete _buffers[i];
            _buffers[i] = nullptr;
        }
    }

    size_t GeometryData::getVideoMemoryFootprint() const {
        size_t sum = 0;

        if (_verticesBuffer != nullptr)
            sum += _verticesBuffer->getBufferSize();
        if (_texCoordsBuffer != nullptr)
            sum += _texCoordsBuffer->getBufferSize();
        if (_colorsBuffer != nullptr)
            sum += _colorsBuffer->getBufferSize();
        if (_normalsBuffer != nullptr)
            sum += _normalsBuffer->getBufferSize();
        if (_pickingBuffer != nullptr)
            sum += _pickingBuffer->getBufferSize();

        return sum;
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

    const tgt::BufferObject* GeometryData::getPickingBuffer() const {
        return _pickingBuffer;
    }

}