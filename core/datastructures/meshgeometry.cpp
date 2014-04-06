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

#include "meshgeometry.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"

#include <algorithm>
#include <list>
#include <utility>

namespace campvis {

    const std::string MeshGeometry::loggerCat_ = "CAMPVis.core.datastructures.MeshGeometry";

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

    size_t MeshGeometry::getLocalMemoryFootprint() const {
        size_t sum = 0;
        for (std::vector<FaceGeometry>::const_iterator it = _faces.begin(); it != _faces.end(); ++it)
            sum += it->getLocalMemoryFootprint();

        if (_verticesBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_texCoordsBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_colorsBuffer != 0)
            sum += sizeof(tgt::BufferObject);
        if (_normalsBuffer != 0)
            sum += sizeof(tgt::BufferObject);

        return sizeof(*this) + sum;
    }

    size_t MeshGeometry::size() const {
        return _faces.size();
    }

    const std::vector<FaceGeometry>& MeshGeometry::getFaces() const {
        return _faces;
    }


    void MeshGeometry::render(GLenum mode) const {
        if (_faces.empty())
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
        LGL_ERROR;

        GLint startIndex = 0;
        for (std::vector<FaceGeometry>::const_iterator it = _faces.begin(); it != _faces.end(); ++it) {
            GLsizei numVertices = static_cast<GLsizei>(it->getVertices().size());
            if (numVertices > 2)
                glDrawArrays(mode, startIndex, numVertices);
            else
                glDrawArrays(GL_LINES, startIndex, numVertices);
            startIndex += numVertices;
        }
        LGL_ERROR;
    }

    void MeshGeometry::createGLBuffers() const {
        if (_buffersDirty) {
            deleteBuffers();

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

#ifdef CAMPVIS_DEBUG
                if (!createTexCoordsBuffer && !(it->getTextureCoordinates().empty()))
                    LWARNING("Presence of texture coordinates in faces not consistent, not generating texture coordinates VBO!");
                if (!createColorsBuffer && !(it->getColors().empty()))
                    LWARNING("Presence of colors in faces not consistent, not generating colors VBO!");
                if (!createNormalsBuffer && !(it->getNormals().empty()))
                    LWARNING("Presence of normals in faces not consistent, not generating normals VBO!");
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
                    if (numVertices == 0)
                        continue;

                    _verticesBuffer->subdata(startIndex * sizeof(tgt::vec3), &(it->getVertices().front()), numVertices * sizeof(tgt::vec3));

                    if (createTexCoordsBuffer)
                        _texCoordsBuffer->subdata(startIndex * sizeof(tgt::vec3), &(it->getTextureCoordinates().front()), numVertices * sizeof(tgt::vec3));
                    if (createColorsBuffer)
                        _colorsBuffer->subdata(startIndex * sizeof(tgt::vec4), &(it->getColors().front()), numVertices * sizeof(tgt::vec4));
                    if (createNormalsBuffer)
                        _normalsBuffer->subdata(startIndex * sizeof(tgt::vec3), &(it->getNormals().front()), numVertices * sizeof(tgt::vec3));
                
                    startIndex += numVertices;
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

        struct VertexComparerEquals {
            VertexComparerEquals(const std::vector<FaceGeometry>& ref, float epsilon)
                : _ref(ref)
                , _epsilon(epsilon)
            {

            };

            bool operator() (const std::pair<size_t, size_t>& left, const std::pair<size_t, size_t>& right) const {
                return (tgt::distance(_ref[left.first].getVertices()[left.second], _ref[right.first].getVertices()[right.second]) < _epsilon);
            };

            const tgt::vec3& operator() (const std::pair<size_t, size_t>& index) const {
                return _ref[index.first].getVertices()[index.second];
            }

            const std::vector<FaceGeometry>& _ref;
            float _epsilon;
        };
    }

    MeshGeometry MeshGeometry::clipAgainstPlane(float p, const tgt::vec3& normal, bool close /*= true*/, float epsilon /*= 1e-8f*/) const {
        std::vector<FaceGeometry> tmp;
        for (std::vector<FaceGeometry>::const_iterator it = _faces.begin(); it != _faces.end(); ++it) {
            FaceGeometry f = it->clipAgainstPlane(p, normal, epsilon);
            if (f.size() > 0)
                tmp.push_back(f);
        }

        if (close) {
            typedef std::pair<size_t, size_t> Vertex;
            std::list< std::pair<Vertex, Vertex> > unsortedEdges;
            std::list< Vertex > sortedVertices;

            // find all edges lying on clip plane
            for (size_t i = 0; i < tmp.size(); ++i) {
                const FaceGeometry& f = tmp[i];
                for (size_t j = 0; j < f.size(); ++j) {
                    if (    distanceToPlane(f.getVertices()[j], p, normal, epsilon) == 0
                         && distanceToPlane(f.getVertices()[(j+1) % f.size()], p, normal, epsilon) == 0)
                    {
                        unsortedEdges.push_back(std::make_pair(std::make_pair(i, j), std::make_pair(i, (j+1) % f.size())));
                        ++j;
                    }
                }
            }

            if (! unsortedEdges.empty()) {
                // sort edges into sortedVertices list
                VertexComparerEquals vce(tmp, epsilon);
                sortedVertices.push_back(unsortedEdges.front().second);
                unsortedEdges.pop_front();
                while (! unsortedEdges.empty()) {
                    bool didWork = false;

                    // look for the edge that has one vertex in common with the last added sorted vertex
                    for (std::list< std::pair<Vertex, Vertex> >::iterator it = unsortedEdges.begin(); it != unsortedEdges.end(); ++it) {
                        if (vce(sortedVertices.back(), it->first)) {
                            sortedVertices.push_back(it->second);
                            unsortedEdges.erase(it);
                            didWork = true;
                            break;
                        }
                        if (vce(sortedVertices.back(), it->second)) {
                            sortedVertices.push_back(it->first);
                            unsortedEdges.erase(it);
                            didWork = true;
                            break;
                        }
                    }

                    // emergency break...
                    if (! didWork)
                        break;
                }

                if (sortedVertices.size() > 2) {
                    // make face ccw
                    tgt::vec3 closingFaceNormal = tgt::normalize(tgt::cross(vce(sortedVertices.front()), vce(*(++sortedVertices.begin()))));

                    if (tgt::dot(normal, closingFaceNormal) < 0)
                        std::reverse(sortedVertices.begin(), sortedVertices.end());
                }

                // build face
                std::vector<tgt::vec3> verts, texCoords, norms;
                std::vector<tgt::vec4> cols;

                for (std::list< Vertex >::iterator it = sortedVertices.begin(); it != sortedVertices.end(); ++it) {
                    verts.push_back(tmp[it->first].getVertices()[it->second]);
                    if (! tmp[it->first].getTextureCoordinates().empty())
                        texCoords.push_back(tmp[it->first].getTextureCoordinates()[it->second]);
                    if (! tmp[it->first].getColors().empty())
                        cols.push_back(tmp[it->first].getColors()[it->second]);
                    if (! tmp[it->first].getNormals().empty())
                        norms.push_back(tmp[it->first].getNormals()[it->second]);
                }
                tmp.push_back(FaceGeometry(verts, texCoords, cols, norms));
            }
        }

        return MeshGeometry(tmp);
    }

    tgt::Bounds MeshGeometry::getWorldBounds() const {
        tgt::Bounds toReturn;
        for (size_t i = 0; i < _faces.size(); ++i)
            for (std::vector<tgt::vec3>::const_iterator it = _faces[i].getVertices().begin(); it != _faces[i].getVertices().end(); ++it)
                toReturn.addPoint(*it);
        return toReturn;
    }

    bool MeshGeometry::hasTextureCoordinates() const {
        bool toReturn = true;
        for (size_t i = 0; i < _faces.size(); ++i) {
            toReturn &= _faces[i].hasTextureCoordinates();
        }
        return toReturn;
    }

    void MeshGeometry::applyTransformationToVertices(const tgt::mat4& t) {
        for (size_t i = 0; i < _faces.size(); ++i) {
            _faces[i].applyTransformationToVertices(t);
        }
    }

}