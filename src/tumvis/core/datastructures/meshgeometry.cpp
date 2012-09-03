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

#include "meshgeometry.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"

namespace TUMVis {

    const std::string MeshGeometry::loggerCat_ = "TUMVis.core.datastructures.MeshGeometry";

    MeshGeometry::MeshGeometry(const std::vector<FaceGeometry>& faces)
        : GeometryData()
        , _faces(faces)
    {
    }

    MeshGeometry::~MeshGeometry() {

    }

    MeshGeometry* MeshGeometry::clone() const {
        return new MeshGeometry(_faces);
    }

    size_t MeshGeometry::size() const {
        return _faces.size();
    }

    const std::vector<FaceGeometry>& MeshGeometry::getFaces() const {
        return _faces;
    }


    void MeshGeometry::render() const {
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

        size_t startIndex = 0;
        for (std::vector<FaceGeometry>::const_iterator it = _faces.begin(); it != _faces.end(); ++it) {
            size_t numVertices = it->getVertices().size();
            if (numVertices > 2)
                glDrawArrays(GL_POLYGON, startIndex, numVertices);
            else
                glDrawArrays(GL_LINES, startIndex, numVertices);
            startIndex += numVertices;
        }
        LGL_ERROR;
    }

    void MeshGeometry::createGLBuffers() const {
        if (! _buffersInitialized) {
            bool createTexCoordsBuffer = true;
            bool createColorsBuffer = true;
            bool createNormalsBuffer = true;

            size_t totalVertices = 0;
            // Check which buffers are to create. Meanwhile calculate the total number of vertices:
            for (std::vector<FaceGeometry>::const_iterator it = _faces.begin(); it != _faces.end(); ++it) {
                totalVertices += it->getVertices().size();

                createTexCoordsBuffer &= !(it->getTextureCoordinates().empty());
                createColorsBuffer &= !(it->getColors().empty());
                createNormalsBuffer &= !(it->getNormals().empty());

#ifdef TUMVIS_DEBUG
                if (!createTexCoordsBuffer && !(it->getTextureCoordinates().empty()))
                    LDEBUG("Presence of texture coordinates in faces not consistend, not generating texture coordinates VBO!");
                if (!createColorsBuffer && !(it->getColors().empty()))
                    LDEBUG("Presence of colors in faces not consistend, not generating colors VBO!");
                if (!createNormalsBuffer && !(it->getNormals().empty()))
                    LDEBUG("Presence of normals in faces not consistend, not generating normals VBO!");
#endif
            }

            try {
                // Now, create all necessary VBOs and reserve data
                _verticesBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                _verticesBuffer->data(0, totalVertices * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);

                if (createTexCoordsBuffer) {
                    _texCoordsBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                    _texCoordsBuffer->data(0, totalVertices * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);
                }
                if (createColorsBuffer) {
                    _colorsBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                    _colorsBuffer->data(0, totalVertices * sizeof(tgt::vec4), tgt::BufferObject::FLOAT, 4);
                }
                if (createNormalsBuffer) {
                    _normalsBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
                    _normalsBuffer->data(0, totalVertices * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);
                }

                // Now start filling the VBOs with data, one face at a time...
                size_t startIndex = 0;
                for (std::vector<FaceGeometry>::const_iterator it = _faces.begin(); it != _faces.end(); ++it) {
                    size_t numVertices = it->getVertices().size();

                    _verticesBuffer->subdata(startIndex * sizeof(tgt::vec3), &(it->getVertices().front()), numVertices * sizeof(tgt::vec3));

                    if (createTexCoordsBuffer)
                        _texCoordsBuffer->subdata(startIndex * sizeof(tgt::vec3), &(it->getTextureCoordinates().front()), numVertices * sizeof(tgt::vec3));
                    if (createColorsBuffer)
                        _colorsBuffer->subdata(startIndex * sizeof(tgt::vec3), &(it->getColors().front()), numVertices * sizeof(tgt::vec3));
                    if (createNormalsBuffer)
                        _normalsBuffer->subdata(startIndex * sizeof(tgt::vec4), &(it->getNormals().front()), numVertices * sizeof(tgt::vec4));
                
                    startIndex += numVertices;
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

    TUMVis::MeshGeometry* MeshGeometry::createCube(const tgt::Bounds& bounds, const tgt::Bounds& texBounds) {
        const tgt::vec3& llf = bounds.getLLF();
        const tgt::vec3& urb = bounds.getURB();
        const tgt::vec3& tLlf = texBounds.getLLF();
        const tgt::vec3& tUrb = texBounds.getURB();

        // not the most elegant method, but it works...
        std::vector<tgt::vec3> vertices, texCoords;
        std::vector<FaceGeometry> faces;

        // front
        texCoords.push_back(tgt::vec3(tLlf.x, tUrb.y, tLlf.z));
        vertices.push_back(tgt::vec3(llf.x, urb.y, llf.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tUrb.y, tLlf.z));
        vertices.push_back(tgt::vec3(urb.x, urb.y, llf.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tLlf.y, tLlf.z));
        vertices.push_back(tgt::vec3(urb.x, llf.y, llf.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tLlf.y, tLlf.z));
        vertices.push_back(tgt::vec3(llf.x, llf.y, llf.z));
        faces.push_back(FaceGeometry(vertices, texCoords));
        vertices.clear();
        texCoords.clear();

        // right
        texCoords.push_back(tgt::vec3(tUrb.x, tUrb.y, tLlf.z));
        vertices.push_back(tgt::vec3(urb.x, urb.y, llf.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tUrb.y, tUrb.z));
        vertices.push_back(tgt::vec3(urb.x, urb.y, urb.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tLlf.y, tUrb.z));
        vertices.push_back(tgt::vec3(urb.x, llf.y, urb.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tLlf.y, tLlf.z));
        vertices.push_back(tgt::vec3(urb.x, llf.y, llf.z));
        faces.push_back(FaceGeometry(vertices, texCoords));
        vertices.clear();
        texCoords.clear();

        // top
        texCoords.push_back(tgt::vec3(tLlf.x, tUrb.y, tUrb.z));
        vertices.push_back(tgt::vec3(llf.x, urb.y, urb.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tUrb.y, tUrb.z));
        vertices.push_back(tgt::vec3(urb.x, urb.y, urb.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tUrb.y, tLlf.z));
        vertices.push_back(tgt::vec3(urb.x, urb.y, llf.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tUrb.y, tLlf.z));
        vertices.push_back(tgt::vec3(llf.x, urb.y, llf.z));
        faces.push_back(FaceGeometry(vertices, texCoords));
        vertices.clear();
        texCoords.clear();

        // left
        texCoords.push_back(tgt::vec3(tLlf.x, tUrb.y, tUrb.z));
        vertices.push_back(tgt::vec3(llf.x, urb.y, urb.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tUrb.y, tLlf.z));
        vertices.push_back(tgt::vec3(llf.x, urb.y, llf.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tLlf.y, tLlf.z));
        vertices.push_back(tgt::vec3(llf.x, llf.y, llf.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tLlf.y, tUrb.z));
        vertices.push_back(tgt::vec3(llf.x, llf.y, urb.z));
        faces.push_back(FaceGeometry(vertices, texCoords));
        vertices.clear();
        texCoords.clear();

        // bottom
        texCoords.push_back(tgt::vec3(tLlf.x, tLlf.y, tLlf.z));
        vertices.push_back(tgt::vec3(llf.x, llf.y, llf.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tLlf.y, tLlf.z));
        vertices.push_back(tgt::vec3(urb.x, llf.y, llf.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tLlf.y, tUrb.z));
        vertices.push_back(tgt::vec3(urb.x, llf.y, urb.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tLlf.y, tUrb.z));
        vertices.push_back(tgt::vec3(llf.x, llf.y, urb.z));
        faces.push_back(FaceGeometry(vertices, texCoords));
        vertices.clear();
        texCoords.clear();

        // back
        texCoords.push_back(tgt::vec3(tUrb.x, tUrb.y, tUrb.z));
        vertices.push_back(tgt::vec3(urb.x, urb.y, urb.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tUrb.y, tUrb.z));
        vertices.push_back(tgt::vec3(llf.x, urb.y, urb.z));
        texCoords.push_back(tgt::vec3(tLlf.x, tLlf.y, tUrb.z));
        vertices.push_back(tgt::vec3(llf.x, llf.y, urb.z));
        texCoords.push_back(tgt::vec3(tUrb.x, tLlf.y, tUrb.z));
        vertices.push_back(tgt::vec3(urb.x, llf.y, urb.z));
        faces.push_back(FaceGeometry(vertices, texCoords));
        vertices.clear();
        texCoords.clear();

        return new MeshGeometry(faces);
    }

}