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
        return new FaceGeometry(_vertices, _textureCoordinates, _colors, _normals);
    }

    size_t FaceGeometry::getLocalMemoryFootprint() const {
        size_t sum = 0;
        if (_verticesBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_texCoordsBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_colorsBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_normalsBuffer != 0)
            sum += sizeof(tgt::BufferObject);

        return sizeof(*this) + sum + (sizeof(tgt::vec3) * (_vertices.size() + _textureCoordinates.size() + _normals.size())) + (sizeof(tgt::vec4) * _colors.size());
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

    const tgt::vec3& FaceGeometry::getFaceNormal() const {
        return _faceNormal;
    }

    void FaceGeometry::render(GLenum mode) const {
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
            }

            lastIndex = i;
            lastDistance = currrentDistance;
        }

        return FaceGeometry(verts, texCoords, cols, norms);
    }

    tgt::Bounds FaceGeometry::getWorldBounds() const {
        tgt::Bounds toReturn;
        for (std::vector<tgt::vec3>::const_iterator it = _vertices.begin(); it != _vertices.end(); ++it)
            toReturn.addPoint(*it);
        return toReturn;
    }

}