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

#include "indexedmeshgeometry.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"

#include <algorithm>
#include <list>
#include <utility>

namespace campvis {

    const std::string IndexedMeshGeometry::loggerCat_ = "CAMPVis.core.datastructures.IndexedMeshGeometry";


    IndexedMeshGeometry::IndexedMeshGeometry(
        const std::vector<uint16_t>& indices, 
        const std::vector<tgt::vec3>& vertices, 
        const std::vector<tgt::vec3>& textureCoordinates /*= std::vector<tgt::vec3>()*/, 
        const std::vector<tgt::vec4>& colors /*= std::vector<tgt::vec4>()*/, 
        const std::vector<tgt::vec3>& normals /*= std::vector<tgt::vec3>() */)
        : GeometryData()
        , _indices(indices)
        , _vertices(vertices)
        , _textureCoordinates(textureCoordinates)
        , _colors(colors)
        , _normals(normals)
        , _indicesBuffer(0)
    {
        tgtAssert(textureCoordinates.empty() || textureCoordinates.size() == vertices.size(), "Texture coordinates vector must be either empty or of the same size as the vertex vector.");
        tgtAssert(colors.empty() || colors.size() == vertices.size(), "Colors vector must be either empty or of the same size as the vertex vector.");
        tgtAssert(normals.empty() || normals.size() == vertices.size(), "Normals vector must be either empty or of the same size as the vertex vector.");

    }

    IndexedMeshGeometry::IndexedMeshGeometry(const IndexedMeshGeometry& rhs)
        : GeometryData(rhs)
        , _indices(rhs._indices)
        , _vertices(rhs._vertices)
        , _textureCoordinates(rhs._textureCoordinates)
        , _colors(rhs._colors)
        , _normals(rhs._normals)
        , _indicesBuffer(0)
    {

    }
    IndexedMeshGeometry::~IndexedMeshGeometry() {
        deleteIndicesBuffer();
    }

    IndexedMeshGeometry& IndexedMeshGeometry::operator=(const IndexedMeshGeometry& rhs) {
        if (this == &rhs)
            return *this;

        GeometryData::operator=(rhs);

        // delete old VBOs and null pointers
        deleteIndicesBuffer();

        return *this;
    }

    IndexedMeshGeometry* IndexedMeshGeometry::clone() const {
        return new IndexedMeshGeometry(_indices, _vertices, _textureCoordinates, _colors, _normals);
    }

    size_t IndexedMeshGeometry::getLocalMemoryFootprint() const {
        size_t sum = 0;

        if (_indicesBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_verticesBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_texCoordsBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_colorsBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_normalsBuffer != 0)
            sum += sizeof(tgt::BufferObject);

        return sizeof(*this) + sum + (sizeof(size_t) * _indices.size()) + (sizeof(tgt::vec3) * (_vertices.size() + _textureCoordinates.size() + _normals.size())) + (sizeof(tgt::vec4) * _colors.size());
    }

    size_t IndexedMeshGeometry::getVideoMemoryFootprint() const {
        return GeometryData::getVideoMemoryFootprint() + (_indicesBuffer == 0 ? 0 : _indicesBuffer->getBufferSize());
    }


    void IndexedMeshGeometry::render(GLenum mode) const {
        createGLBuffers();
        if (_buffersDirty) {
            LERROR("Cannot render without initialized OpenGL buffers.");
            return;
        }

        tgt::VertexArrayObject vao;
        if (_verticesBuffer)
            vao.setVertexAttributePointer(0, _verticesBuffer);
        if (_texCoordsBuffer)
            vao.setVertexAttributePointer(1, _texCoordsBuffer);
        if (_colorsBuffer)
            vao.setVertexAttributePointer(2, _colorsBuffer);
        if (_normalsBuffer)
            vao.setVertexAttributePointer(3, _normalsBuffer);
        vao.bindIndexBuffer(_indicesBuffer);

        glDrawElements(mode, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_SHORT, 0);
        LGL_ERROR;
    }

    void IndexedMeshGeometry::createGLBuffers() const {
        if (_buffersDirty) {
            deleteBuffers();
            deleteIndicesBuffer();

            try {
                _indicesBuffer = new tgt::BufferObject(tgt::BufferObject::ELEMENT_ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                _indicesBuffer->data(&_indices.front(), _indices.size() * sizeof(uint16_t), tgt::BufferObject::UNSIGNED_SHORT, 1);

                _verticesBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                _verticesBuffer->data(&_vertices.front(), _vertices.size() * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);

                if (! _textureCoordinates.empty()) {
                    _texCoordsBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                    _texCoordsBuffer->data(&_textureCoordinates.front(), _textureCoordinates.size() * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);
                }
                if (! _colors.empty()) {
                    _colorsBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                    _colorsBuffer->data(&_colors.front(), _colors.size() * sizeof(tgt::vec4), tgt::BufferObject::FLOAT, 4);
                }
                if (! _normals.empty()) {
                    _normalsBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                    _normalsBuffer->data(&_normals.front(), _normals.size() * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);
                }
            }
            catch (tgt::Exception& e) {
                LERROR("Error creating OpenGL Buffer objects: " << e.what());
                _buffersDirty = true;
                return;
            }

            LGL_ERROR;
            _buffersDirty = false;
        }
    }

    tgt::Bounds IndexedMeshGeometry::getWorldBounds() const {
        tgt::Bounds toReturn;
        for (std::vector<tgt::vec3>::const_iterator it = _vertices.begin(); it != _vertices.end(); ++it)
            toReturn.addPoint(*it);
        return toReturn;
    }
    
    void IndexedMeshGeometry::deleteIndicesBuffer() const {
        delete _indicesBuffer;
        _indicesBuffer = 0;
    }


}