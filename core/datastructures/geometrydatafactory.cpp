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

#include "geometrydatafactory.h"

#include "tgt/assert.h"

#include "core/tools/teapot.h"

namespace campvis {

    FaceGeometry* GeometryDataFactory::createQuad(const tgt::vec3& llf, const tgt::vec3& urb, const tgt::vec3& texLlf, const tgt::vec3& texUrb) {
        std::vector<tgt::vec3> vertices, texCorods;

        vertices.push_back(tgt::vec3(llf.x, llf.y, llf.z));
        vertices.push_back(tgt::vec3(urb.x, llf.y, llf.z));
        vertices.push_back(tgt::vec3(urb.x, urb.y, llf.z));
        vertices.push_back(tgt::vec3(llf.x, urb.y, llf.z));

        texCorods.push_back(tgt::vec3(texLlf.x, texLlf.y, texLlf.z));
        texCorods.push_back(tgt::vec3(texUrb.x, texLlf.y, texLlf.z));
        texCorods.push_back(tgt::vec3(texUrb.x, texUrb.y, texLlf.z));
        texCorods.push_back(tgt::vec3(texLlf.x, texUrb.y, texLlf.z));

        return new FaceGeometry(vertices, texCorods);
    }

    MeshGeometry* GeometryDataFactory::createCube(const tgt::Bounds& bounds, const tgt::Bounds& texBounds) {
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
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<tgt::vec4>(), std::vector<tgt::vec3>(4, tgt::vec3(0.f, 0.f, -1.f))));
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
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<tgt::vec4>(), std::vector<tgt::vec3>(4, tgt::vec3(1.f, 0.f, 0.f))));
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
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<tgt::vec4>(), std::vector<tgt::vec3>(4, tgt::vec3(0.f, 1.f, 0.f))));
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
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<tgt::vec4>(), std::vector<tgt::vec3>(4, tgt::vec3(-1.f, 0.f, 0.f))));
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
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<tgt::vec4>(), std::vector<tgt::vec3>(4, tgt::vec3(0.f, -1.f, 0.f))));
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
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<tgt::vec4>(), std::vector<tgt::vec3>(4, tgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        return new MeshGeometry(faces);
    }

    MultiIndexedGeometry* GeometryDataFactory::createTeapot() {
        std::vector<tgt::vec3> vertices, normals;
        vertices.reserve(Teapot::num_teapot_vertices);
        normals.reserve(Teapot::num_teapot_vertices);

        for (size_t i = 0; i < Teapot::num_teapot_vertices; ++i) {
            vertices.push_back(tgt::vec3(Teapot::teapot_vertices + 3*i));
            normals.push_back(tgt::vec3(Teapot::teapot_normals + 3*i));
        }

        MultiIndexedGeometry* toReturn = new MultiIndexedGeometry(vertices, std::vector<tgt::vec3>(), std::vector<tgt::vec4>(), normals);

        // convert indices and add primitives 
        int currentOffset = 0;
        while (currentOffset < Teapot::num_teapot_indices) {
            uint16_t count = Teapot::new_teapot_indicies[currentOffset];
            toReturn->addPrimitive(std::vector<uint16_t>(Teapot::new_teapot_indicies + currentOffset + 1, Teapot::new_teapot_indicies + count + currentOffset + 1));
            currentOffset += count + 1;
        }

        return toReturn;
    }

    MultiIndexedGeometry* GeometryDataFactory::createSphere(uint16_t numStacks /*= 6*/, uint16_t numSlices /*= 12*/) {
        tgtAssert(numStacks > 1 && numSlices > 2, "Sphere must have minimum 2 stacks and 3 slices!");
        std::vector<tgt::vec3> vertices;

        // add top vertex
        vertices.push_back(tgt::vec3(0.f, 0.f, 1.f));

        // add middle vertices
        for (int i = 1; i < numStacks; ++i) {
            float phi = static_cast<float>(i) * tgt::PIf / static_cast<float>(numStacks);

            for (int j = 0; j < numSlices; ++j) {
                float theta = static_cast<float>(j) * 2.f*tgt::PIf / static_cast<float>(numSlices);
                vertices.push_back(tgt::vec3(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi)));
            }
        }

        // add bottom vertex
        vertices.push_back(tgt::vec3(0.f, 0.f, -1.f));

        // create geometry (in a unit sphere vertices = normals)
        MultiIndexedGeometry* toReturn = new MultiIndexedGeometry(vertices, std::vector<tgt::vec3>(), std::vector<tgt::vec4>(), vertices);

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

}