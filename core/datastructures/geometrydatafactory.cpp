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