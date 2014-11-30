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

#include "geometrydatafactory.h"

#include "cgt/assert.h"

#include "core/tools/teapot.h"

namespace campvis {

    FaceGeometry* GeometryDataFactory::createQuad(const cgt::vec3& llf, const cgt::vec3& urb, const cgt::vec3& texLlf, const cgt::vec3& texUrb) {
        std::vector<cgt::vec3> vertices, texCorods;

        vertices.push_back(cgt::vec3(llf.x, llf.y, llf.z));
        vertices.push_back(cgt::vec3(urb.x, llf.y, llf.z));
        vertices.push_back(cgt::vec3(urb.x, urb.y, llf.z));
        vertices.push_back(cgt::vec3(llf.x, urb.y, llf.z));

        texCorods.push_back(cgt::vec3(texLlf.x, texLlf.y, texLlf.z));
        texCorods.push_back(cgt::vec3(texUrb.x, texLlf.y, texLlf.z));
        texCorods.push_back(cgt::vec3(texUrb.x, texUrb.y, texLlf.z));
        texCorods.push_back(cgt::vec3(texLlf.x, texUrb.y, texLlf.z));

        return new FaceGeometry(vertices, texCorods);
    }

    MultiIndexedGeometry* GeometryDataFactory::createGrid(const cgt::vec3& llf, const cgt::vec3& urb, const cgt::vec3& texLlf, const cgt::vec3& texUrb, int xSegments, int ySegments) {
        cgtAssert(xSegments > 0 && ySegments > 1, "Grid must have at least one segment in each direction");

        int numVertices = (xSegments + 1) * (ySegments + 1);
        std::vector<cgt::vec3> vertices(numVertices);
        std::vector<cgt::vec3> textureCoordinates(numVertices);
        std::vector<cgt::vec3> normals(numVertices);

        // Compute vertices of the grid in x-major order
        for (int y = 0; y <= ySegments; ++y) {
            for (int x = 0; x <= xSegments; ++x) {
                int idx = y * (xSegments + 1) + x;

                float ux = x / static_cast<float>(xSegments);
                float uy = y / static_cast<float>(ySegments);

                vertices[idx] = cgt::vec3(llf.x * (1-ux) + urb.x * ux,
                                          llf.y * (1-uy) + urb.y * uy,
                                          llf.z);
                textureCoordinates[idx] = cgt::vec3(texLlf.x * (1-ux) + texUrb.x * ux,
                                                    texLlf.y * (1-uy) + texUrb.y * uy,
                                                    texLlf.z);
                normals[idx] = cgt::vec3(0, 0, 1);
            }
        }

        MultiIndexedGeometry* result = new MultiIndexedGeometry(vertices, textureCoordinates, std::vector<cgt::vec4>(), normals);

        // For each horizontal stripe, construct the indeces for triangle strips
        int verticesPerStrip = (xSegments + 1) * 2;
        for (int y = 0; y < ySegments; ++y) {
            std::vector<uint16_t> indices(verticesPerStrip);
            for (uint16_t x = 0; x <= xSegments; ++x) {
                indices[x*2 + 0] = (y + 0) * (xSegments + 1) + x;
                indices[x*2 + 1] = (y + 1) * (xSegments + 1) + x;
            }

            result->addPrimitive(indices);
        }

        return result;
    }


    MeshGeometry* GeometryDataFactory::createCube(const cgt::Bounds& bounds, const cgt::Bounds& texBounds) {
        const cgt::vec3& llf = bounds.getLLF();
        const cgt::vec3& urb = bounds.getURB();
        const cgt::vec3& tLlf = texBounds.getLLF();
        const cgt::vec3& tUrb = texBounds.getURB();

        // not the most elegant method, but it works...
        std::vector<cgt::vec3> vertices, texCoords;
        std::vector<FaceGeometry> faces;

        // front
        texCoords.push_back(cgt::vec3(tLlf.x, tUrb.y, tLlf.z));
        vertices.push_back(cgt::vec3(llf.x, urb.y, llf.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tUrb.y, tLlf.z));
        vertices.push_back(cgt::vec3(urb.x, urb.y, llf.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tLlf.y, tLlf.z));
        vertices.push_back(cgt::vec3(urb.x, llf.y, llf.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tLlf.y, tLlf.z));
        vertices.push_back(cgt::vec3(llf.x, llf.y, llf.z));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, -1.f))));
        vertices.clear();
        texCoords.clear();

        // right
        texCoords.push_back(cgt::vec3(tUrb.x, tUrb.y, tLlf.z));
        vertices.push_back(cgt::vec3(urb.x, urb.y, llf.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tUrb.y, tUrb.z));
        vertices.push_back(cgt::vec3(urb.x, urb.y, urb.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tLlf.y, tUrb.z));
        vertices.push_back(cgt::vec3(urb.x, llf.y, urb.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tLlf.y, tLlf.z));
        vertices.push_back(cgt::vec3(urb.x, llf.y, llf.z));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(), std::vector<cgt::vec3>(4, cgt::vec3(1.f, 0.f, 0.f))));
        vertices.clear();
        texCoords.clear();

        // top
        texCoords.push_back(cgt::vec3(tLlf.x, tUrb.y, tUrb.z));
        vertices.push_back(cgt::vec3(llf.x, urb.y, urb.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tUrb.y, tUrb.z));
        vertices.push_back(cgt::vec3(urb.x, urb.y, urb.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tUrb.y, tLlf.z));
        vertices.push_back(cgt::vec3(urb.x, urb.y, llf.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tUrb.y, tLlf.z));
        vertices.push_back(cgt::vec3(llf.x, urb.y, llf.z));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 1.f, 0.f))));
        vertices.clear();
        texCoords.clear();

        // left
        texCoords.push_back(cgt::vec3(tLlf.x, tUrb.y, tUrb.z));
        vertices.push_back(cgt::vec3(llf.x, urb.y, urb.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tUrb.y, tLlf.z));
        vertices.push_back(cgt::vec3(llf.x, urb.y, llf.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tLlf.y, tLlf.z));
        vertices.push_back(cgt::vec3(llf.x, llf.y, llf.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tLlf.y, tUrb.z));
        vertices.push_back(cgt::vec3(llf.x, llf.y, urb.z));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(), std::vector<cgt::vec3>(4, cgt::vec3(-1.f, 0.f, 0.f))));
        vertices.clear();
        texCoords.clear();

        // bottom
        texCoords.push_back(cgt::vec3(tLlf.x, tLlf.y, tLlf.z));
        vertices.push_back(cgt::vec3(llf.x, llf.y, llf.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tLlf.y, tLlf.z));
        vertices.push_back(cgt::vec3(urb.x, llf.y, llf.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tLlf.y, tUrb.z));
        vertices.push_back(cgt::vec3(urb.x, llf.y, urb.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tLlf.y, tUrb.z));
        vertices.push_back(cgt::vec3(llf.x, llf.y, urb.z));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(), std::vector<cgt::vec3>(4, cgt::vec3(0.f, -1.f, 0.f))));
        vertices.clear();
        texCoords.clear();

        // back
        texCoords.push_back(cgt::vec3(tUrb.x, tUrb.y, tUrb.z));
        vertices.push_back(cgt::vec3(urb.x, urb.y, urb.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tUrb.y, tUrb.z));
        vertices.push_back(cgt::vec3(llf.x, urb.y, urb.z));
        texCoords.push_back(cgt::vec3(tLlf.x, tLlf.y, tUrb.z));
        vertices.push_back(cgt::vec3(llf.x, llf.y, urb.z));
        texCoords.push_back(cgt::vec3(tUrb.x, tLlf.y, tUrb.z));
        vertices.push_back(cgt::vec3(urb.x, llf.y, urb.z));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        return new MeshGeometry(faces);
    }

    MultiIndexedGeometry* GeometryDataFactory::createTeapot() {
        std::vector<cgt::vec3> vertices, normals;
        vertices.reserve(Teapot::num_teapot_vertices);
        normals.reserve(Teapot::num_teapot_vertices);

        for (size_t i = 0; i < Teapot::num_teapot_vertices; ++i) {
            vertices.push_back(cgt::vec3(Teapot::teapot_vertices + 3*i));
            normals.push_back(cgt::vec3(Teapot::teapot_normals + 3*i));
        }

        MultiIndexedGeometry* toReturn = new MultiIndexedGeometry(vertices, std::vector<cgt::vec3>(), std::vector<cgt::vec4>(), normals);

        // convert indices and add primitives 
        int currentOffset = 0;
        while (currentOffset < Teapot::num_teapot_indices) {
            uint16_t count = Teapot::new_teapot_indicies[currentOffset];
            toReturn->addPrimitive(std::vector<uint16_t>(Teapot::new_teapot_indicies + currentOffset + 1, Teapot::new_teapot_indicies + count + currentOffset + 1));
            currentOffset += count + 1;
        }

        return toReturn;
    }

    MultiIndexedGeometry* GeometryDataFactory::createSphere(uint16_t numStacks /*= 6*/, uint16_t numSlices /*= 12*/, const cgt::vec3& exponents /*= cgt::vec3(1.f)*/) {
        cgtAssert(numStacks > 1 && numSlices > 2, "Sphere must have minimum 2 stacks and 3 slices!");
        std::vector<cgt::vec3> vertices;
        std::vector<cgt::vec3> textureCoordinates;

        // add top vertex
        vertices.push_back(cgt::vec3(0.f, 0.f, 1.f));
        textureCoordinates.push_back(cgt::vec3(0.f));

        // add middle vertices
        for (int i = 1; i < numStacks; ++i) {
            float phi = static_cast<float>(i) * cgt::PIf / static_cast<float>(numStacks);

            for (int j = 0; j < numSlices; ++j) {
                float theta = static_cast<float>(j) * 2.f*cgt::PIf / static_cast<float>(numSlices);

                // apply exponents for supersphere
                cgt::vec3 theVertex(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
                for (size_t e = 0; e < 3; ++e) {
                    if (theVertex[e] < 0)
                        theVertex[e] = -pow(-theVertex[e], exponents[e]);
                    else
                        theVertex[e] = pow(theVertex[e], exponents[e]);
                }

                vertices.push_back(theVertex);
                textureCoordinates.push_back(cgt::vec3(theta / (2.f * cgt::PIf), phi / cgt::PIf, 0.f));
            }
        }

        // add bottom vertex
        vertices.push_back(cgt::vec3(0.f, 0.f, -1.f));
        textureCoordinates.push_back(cgt::vec3(1.f, 0.f, 0.f));

        // create geometry (in a unit sphere vertices = normals)
        MultiIndexedGeometry* toReturn = new MultiIndexedGeometry(vertices, textureCoordinates, std::vector<cgt::vec4>(), vertices);

        // add indices for primitives to geometry:
        {
            // top stack:
            std::vector<uint16_t> indices;
            for (uint16_t j = 0; j < numSlices; ++j) {
                indices.push_back(0);
                indices.push_back(j+1);
            }
            indices.push_back(0);
            indices.push_back(1);

            toReturn->addPrimitive(indices);
        }
        {
            // middle stacks:
            std::vector<uint16_t> indices;
            for (uint16_t i = 1; i < numStacks-1; ++i) {
                uint16_t startIndex = 1 + (i-1) * numSlices;

                for (uint16_t j = 0; j < numSlices; ++j) {
                    indices.push_back(startIndex + j);
                    indices.push_back(startIndex + numSlices + j);
                }
                indices.push_back(startIndex);
                indices.push_back(startIndex + numSlices);
            }

            toReturn->addPrimitive(indices);
        }
        {
            // bottom stack:
            std::vector<uint16_t> indices;
            uint16_t endIndex = static_cast<uint16_t>(vertices.size() - 1);

            for (uint16_t j = 0; j < numSlices; ++j) {
                indices.push_back(endIndex);
                indices.push_back(endIndex - (j+1));
            }
            indices.push_back(endIndex);
            indices.push_back(endIndex - 1);

            toReturn->addPrimitive(indices);
        }

        return toReturn;
    }

    MultiIndexedGeometry* GeometryDataFactory::createArrow(uint16_t numSlices, float tipLen, float cylRadius, float tipRadius) {
        cgtAssert(numSlices > 2, "Arrow shaft must have minimum 3 slices!");
        cgtAssert(tipRadius > cylRadius, "Tip radius must exceed cyclinder radius (for correct normals)!");
        cgtAssert(tipLen > 0, "Tip length must be between 0 and 1!");
        cgtAssert(tipLen < 1, "Tip length must be between 0 and 1!");
        std::vector<cgt::vec3> vertices;
        std::vector<cgt::vec3> normals;

        // add bottom vertex
        vertices.push_back(cgt::vec3(0.f, 0.f, 0.f));
        normals.push_back(cgt::vec3(0.f, 0.f, -1.f));

        // add shaft floor vertices
        for (int i = 0; i < numSlices; ++i) {
            float theta = static_cast<float>(i) * 2.f*cgt::PIf / static_cast<float>(numSlices);
            vertices.push_back(cgt::vec3(cylRadius * cos(theta), cylRadius * sin(theta), 0.f));
            normals.push_back(cgt::vec3(0.f, 0.f, -1.f));
        }
        for (int i = 0; i < numSlices; ++i) {
            float theta = static_cast<float>(i) * 2.f*cgt::PIf / static_cast<float>(numSlices);
            vertices.push_back(cgt::vec3(cylRadius * cos(theta), cylRadius * sin(theta), 0.f));
            normals.push_back(cgt::vec3(cos(theta), sin(theta), 0.f));
        }

        // add shaft top vertices
        for (int i = 0; i < numSlices; ++i) {
            float theta = static_cast<float>(i) * 2.f*cgt::PIf / static_cast<float>(numSlices);
            vertices.push_back(cgt::vec3(cylRadius * cos(theta), cylRadius * sin(theta), 1.f - tipLen));
            normals.push_back(cgt::vec3(cos(theta), sin(theta), 0.f));
        }
        for (int i = 0; i < numSlices; ++i) {
            float theta = static_cast<float>(i) * 2.f*cgt::PIf / static_cast<float>(numSlices);
            vertices.push_back(cgt::vec3(cylRadius * cos(theta), cylRadius * sin(theta), 1.f - tipLen));
            normals.push_back(cgt::vec3(0.f, 0.f, -1.f));
        }

        // add arrow tip outer cone vertices
        for (int i = 0; i < numSlices; ++i) {
            float theta = static_cast<float>(i) * 2.f*cgt::PIf / static_cast<float>(numSlices);
            vertices.push_back(cgt::vec3(tipRadius * cos(theta), tipRadius * sin(theta), 1.f - tipLen));
            normals.push_back(cgt::vec3(0.f, 0.f, -1.f));
        }
        float phi = atan2f(tipRadius, tipLen);
        for (int i = 0; i < numSlices; ++i) {
            float theta = static_cast<float>(i) * 2.f*cgt::PIf / static_cast<float>(numSlices);
            vertices.push_back(cgt::vec3(tipRadius * cos(theta), tipRadius * sin(theta), 1.f - tipLen));
            normals.push_back(cgt::vec3(cos(theta) * cos(phi), sin(theta) * cos(phi), sin(phi)));
        }

        // add top vertex
        vertices.push_back(cgt::vec3(0.f, 0.f, 1.f));
        normals.push_back(cgt::vec3(0.f, 0.f, 1.f));

        // create geometry
        MultiIndexedGeometry* toReturn = new MultiIndexedGeometry(vertices, std::vector<cgt::vec3>(), std::vector<cgt::vec4>(), normals);

        // add indices for primitives to geometry:
        {
            // cylinder floor
            std::vector<uint16_t> indices;
            for (uint16_t j = 0; j < numSlices; ++j) {
                indices.push_back(0);
                indices.push_back(j+1);
            }
            indices.push_back(0);
            indices.push_back(1);

            toReturn->addPrimitive(indices);
        }
        {
            // cylinder shaft
            std::vector<uint16_t> indices;
            for (uint16_t j = 0; j < numSlices; ++j) {
                indices.push_back(j+1+numSlices);
                indices.push_back(j+1+numSlices*2);
            }
            indices.push_back(1+numSlices);
            indices.push_back(1+numSlices*2);

            toReturn->addPrimitive(indices);
        }
        {
            // arrow tip bottom area
            std::vector<uint16_t> indices;
            for (uint16_t j = 0; j < numSlices; ++j) {
                indices.push_back(j+1+numSlices*3);
                indices.push_back(j+1+numSlices*4);
            }
            indices.push_back(1+numSlices*3);
            indices.push_back(1+numSlices*4);

            toReturn->addPrimitive(indices);
        }
        {
            // arrow tip cone
            uint16_t m = static_cast<uint16_t>(vertices.size() - 1);
            std::vector<uint16_t> indices;
            for (uint16_t j = 0; j < numSlices; ++j) {
                indices.push_back(j+1+numSlices*5);
                indices.push_back(m);
            }
            indices.push_back(1+numSlices*5);
            indices.push_back(m);

            toReturn->addPrimitive(indices);
        }

        return toReturn;
    }

}