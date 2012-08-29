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

#include "facegeometry.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"

namespace TUMVis {

    const std::string FaceGeometry::loggerCat_ = "TUMVis.core.datastructures.FaceGeometry";

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

    void FaceGeometry::render() {
        createGLBuffers();
        if (! _buffersInitialized) {
            LERROR("Cannot render without initialized OpenGL buffers.");
            return;
        }

        tgt::VertexArrayObject vao;
        if (_verticesBuffer)
            vao.addVertexAttribute(tgt::VertexArrayObject::VerticesAttribute, _verticesBuffer);
        if (_texCoordsBuffer)
            vao.addVertexAttribute(tgt::VertexArrayObject::TextureCoordinatesAttribute, _texCoordsBuffer);
        if (_colorsBuffer)
            vao.addVertexAttribute(tgt::VertexArrayObject::ColorsAttribute, _colorsBuffer);
        if (_normalsBuffer)
            vao.addVertexAttribute(tgt::VertexArrayObject::NormalsAttribute, _normalsBuffer);
        LGL_ERROR;

        if (_vertices.size() > 2)
            glDrawArrays(GL_POLYGON, 0, _vertices.size());
        else
            glDrawArrays(GL_LINES, 0, _vertices.size());
        LGL_ERROR;
    }

    void FaceGeometry::createGLBuffers() {
        if (! _buffersInitialized) {
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
                _buffersInitialized = false;
                return;
            }

            LGL_ERROR;
            _buffersInitialized = true;
        }
    }

}