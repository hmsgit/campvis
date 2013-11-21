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