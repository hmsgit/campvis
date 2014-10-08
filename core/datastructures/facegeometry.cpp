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

#include "facegeometry.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"
#include "tgt/vector.h"

namespace campvis {

    const std::string FaceGeometry::loggerCat_ = "CAMPVis.core.datastructures.FaceGeometry";

    FaceGeometry::FaceGeometry()
        : GeometryData()
        , _faceNormal(0.f)
    {

    }

    FaceGeometry::FaceGeometry(const std::vector<tgt::vec3>& vertices, const std::vector<tgt::vec3>& textureCoordinates /*= std::vector<tgt::vec3>()*/, const std::vector<tgt::vec4>& colors /*= std::vector<tgt::vec4>()*/, const std::vector<tgt::vec3>& normals /*= std::vector<tgt::vec3>() */)
        : GeometryData()
        , _vertices(vertices)
        , _textureCoordinates(textureCoordinates)
        , _colors(colors)
        , _normals(normals)
        , _faceNormal(0.f)
    {
        tgtAssert(textureCoordinates.empty() || textureCoordinates.size() == vertices.size(), "Texture coordinates vector must be either empty or of the same size as the vertex vector.");
        tgtAssert(colors.empty() || colors.size() == vertices.size(), "Colors vector must be either empty or of the same size as the vertex vector.");
        tgtAssert(normals.empty() || normals.size() == vertices.size(), "Normals vector must be either empty or of the same size as the vertex vector.");

        // compute normal (class invariant states, that all vertices lie in the same plane):
        if (_vertices.size() > 2) {
            _faceNormal = tgt::normalize(tgt::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
        }
    }

    FaceGeometry::~FaceGeometry() {

    }

    FaceGeometry* FaceGeometry::clone() const {
        FaceGeometry* toReturn = new FaceGeometry(_vertices, _textureCoordinates, _colors, _normals);
        toReturn->setPickingInformation(_pickingInformation);
        return toReturn;
    }

    size_t FaceGeometry::getLocalMemoryFootprint() const {
        size_t sum = 0;
        for (size_t i = 0; i < NUM_BUFFERS; ++i) {
            if (_buffers[i] != nullptr)
                sum += sizeof(tgt::BufferObject);
        }

        return sizeof(*this) + sum + (sizeof(tgt::vec3) * (_vertices.size() + _textureCoordinates.size() + _normals.size())) + (sizeof(tgt::vec4) * (_colors.size() + _pickingInformation.size()));
    }

    size_t FaceGeometry::size() const {
        return _vertices.size();
    }

    const std::vector<tgt::vec3>& FaceGeometry::getVertices() const {
        return _vertices;
    }

    const std::vector<tgt::vec4>& FaceGeometry::getColors() const {
        return _colors;
    }

    const std::vector<tgt::vec3>& FaceGeometry::getNormals() const {
        return _normals;
    }

    const std::vector<tgt::vec3>& FaceGeometry::getTextureCoordinates() const {
        return _textureCoordinates;
    }

    const std::vector<tgt::col4>& FaceGeometry::getPickingInformation() const {
        return _pickingInformation;
    }

    void FaceGeometry::setPickingInformation(const std::vector<tgt::col4>& pickingInformation) {
        tgtAssert(pickingInformation.size() == 0 || pickingInformation.size() == _vertices.size(), "Number of picking informations does not match number of vertices!");
        _pickingInformation = pickingInformation;
        _buffersDirty = true;
    }

    const tgt::vec3& FaceGeometry::getFaceNormal() const {
        return _faceNormal;
    }

    void FaceGeometry::render(GLenum mode) const {
        if (_vertices.empty())
            return;

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
        if (_pickingBuffer)
            vao.setVertexAttributePointer(4, _pickingBuffer);
        LGL_ERROR;

        glDrawArrays(mode, 0, static_cast<GLsizei>(_vertices.size()));
        LGL_ERROR;
    }

    void FaceGeometry::createGLBuffers() const {
        if (_buffersDirty) {
            deleteBuffers();

            try {
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
                if (! _pickingInformation.empty()) {
                    _pickingBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                    _pickingBuffer->data(&_pickingInformation.front(), _pickingInformation.size() * sizeof(tgt::col4), tgt::BufferObject::UNSIGNED_BYTE, 4);
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

    namespace {
        float distanceToPlane(const tgt::vec3& vertex, float p, const tgt::vec3& pNormal, float epsilon) {
            float distance = tgt::dot(pNormal, vertex) - p;
            if (std::abs(distance) <= epsilon)
                return 0;
            else
                return distance;
        }
    }

    campvis::FaceGeometry FaceGeometry::clipAgainstPlane(float p, const tgt::vec3& pNormal, float epsilon /*= 1e-4f*/) const {
        tgtAssert(epsilon >= 0, "Epsilon must be positive.");

        std::vector<tgt::vec3> verts, texCoords, norms;
        std::vector<tgt::vec4> cols;
        std::vector<tgt::col4> picks;
        size_t lastIndex = _vertices.size() - 1;
        float lastDistance = distanceToPlane(_vertices.back(), p, pNormal, epsilon);

        // Implementation of Sutherland-Hodgman polygon clipping:
        for (size_t i = 0; i < _vertices.size(); ++i) {
            float currrentDistance = distanceToPlane(_vertices[i], p, pNormal, epsilon);

            // case 1: last vertex outside, this vertex inside clip region => clip
            if (lastDistance > 0 && currrentDistance <= 0) {
                float t = lastDistance / (lastDistance - currrentDistance);
                
                verts.push_back(tgt::mix(_vertices[lastIndex], _vertices[i], t));
                if (!_textureCoordinates.empty())
                    texCoords.push_back(tgt::mix(_textureCoordinates[lastIndex], _textureCoordinates[i], t));
                if (!_colors.empty())
                    cols.push_back(tgt::mix(_colors[lastIndex], _colors[i], t));
                if (!_normals.empty())
                    norms.push_back(tgt::mix(_normals[lastIndex], _normals[i], t));
                if (!_pickingInformation.empty())
                    picks.push_back(_pickingInformation[i]);
            }
            // case 2: last vertex inside, this vertex outside clip region => clip
            else if (lastDistance <= 0 && currrentDistance > 0) {
                float t = lastDistance / (lastDistance - currrentDistance);

                verts.push_back(tgt::mix(_vertices[lastIndex], _vertices[i], t));
                if (!_textureCoordinates.empty())
                    texCoords.push_back(tgt::mix(_textureCoordinates[lastIndex], _textureCoordinates[i], t));
                if (!_colors.empty())
                    cols.push_back(tgt::mix(_colors[lastIndex], _colors[i], t));
                if (!_normals.empty())
                    norms.push_back(tgt::mix(_normals[lastIndex], _normals[i], t));
                if (!_pickingInformation.empty())
                    picks.push_back(_pickingInformation[lastIndex]);
            }

            // case 1.2 + case 3: current vertix in front of plane => keep
            if (currrentDistance <= 0) {
                verts.push_back(_vertices[i]);
                if (!_textureCoordinates.empty())
                    texCoords.push_back(_textureCoordinates[i]);
                if (!_colors.empty())
                    cols.push_back(_colors[i]);
                if (!_normals.empty())
                    norms.push_back(_normals[i]);
                if (!_pickingInformation.empty())
                    picks.push_back(_pickingInformation[i]);
            }

            lastIndex = i;
            lastDistance = currrentDistance;
        }

        FaceGeometry toReturn(verts, texCoords, cols, norms);
        toReturn.setPickingInformation(picks);
        return toReturn;
    }

    tgt::Bounds FaceGeometry::getWorldBounds() const {
        tgt::Bounds toReturn;
        for (std::vector<tgt::vec3>::const_iterator it = _vertices.begin(); it != _vertices.end(); ++it)
            toReturn.addPoint(*it);
        return toReturn;
    }

    bool FaceGeometry::hasTextureCoordinates() const {
        return ! _textureCoordinates.empty();
    }

    bool FaceGeometry::hasPickingInformation() const {
        return !_pickingInformation.empty();
    }

    void FaceGeometry::applyTransformationToVertices(const tgt::mat4& t) {
        for (size_t i = 0; i < _vertices.size(); ++i) {
            tgt::vec4 tmp = t * tgt::vec4(_vertices[i], 1.f);
            _vertices[i] = tmp.xyz() / tmp.w;
        }

        _buffersDirty = true;
    }


}