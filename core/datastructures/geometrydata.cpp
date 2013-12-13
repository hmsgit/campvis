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
        , _verticesBuffer(0)
        , _texCoordsBuffer(0)
        , _colorsBuffer(0)
        , _normalsBuffer(0)
    {
    }

    GeometryData::GeometryData(const GeometryData& rhs)
        : AbstractData(rhs)
        , _buffersDirty(true)
        , _verticesBuffer(0)
        , _texCoordsBuffer(0)
        , _colorsBuffer(0)
        , _normalsBuffer(0)
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
            _buffers[i] = 0;
        }
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