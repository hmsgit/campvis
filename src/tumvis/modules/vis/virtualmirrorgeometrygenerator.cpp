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

#include "virtualmirrorgeometrygenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/meshgeometry.h"

namespace TUMVis {
    const std::string VirtualMirrorGeometryGenerator::loggerCat_ = "TUMVis.modules.vis.VirtualMirrorGeometryGenerator";

    VirtualMirrorGeometryGenerator::VirtualMirrorGeometryGenerator()
        : AbstractProcessor()
        , _mirrorID("geometryID", "Output Geometry ID", "mirror", DataNameProperty::WRITE)
        , _mirrorCenter("mirrorCenter", "Mirror Center", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f))
        , _mirrorNormal("mirrorNormal", "Mirror Normal", tgt::vec3(0.f, 1.f, 0.f), tgt::vec3(-1.f), tgt::vec3(1.f), InvalidationLevel::VALID)
        , _size("mirrorSize", "Mirror Size", 4.0f, 0.1f, 1000.0f)
        , _numVertices("Roundness", "Roundness", 16, 4, 128)
        , _poi("poi", "Point of Interest", tgt::vec3::zero, tgt::vec3(-100.f), tgt::vec3(1000.f))
        , _camera("camera", "Camera")
    {
        addProperty(&_mirrorID);
        addProperty(&_mirrorCenter);
        addProperty(&_mirrorNormal);
        addProperty(&_size);
        addProperty(&_numVertices);
        addProperty(&_poi);
        addProperty(&_camera);
    }

    VirtualMirrorGeometryGenerator::~VirtualMirrorGeometryGenerator() {

    }

    void VirtualMirrorGeometryGenerator::process(DataContainer& data) {
        // mirror normal is given by halfway vector between Camera-Mirror and POI-Mirror:
        const tgt::Camera& cam = _camera.getValue();
        tgt::vec3 v1 = tgt::normalize(_poi.getValue() - _mirrorCenter.getValue());
        tgt::vec3 v2 = tgt::normalize(cam.getPosition() - _mirrorCenter.getValue());
        tgt::vec3 n = tgt::normalize(v1 + v2);

        // assure that mirror is faced to camera
        if (tgt::sign(tgt::dot(cam.getLook(), n)) == 1)
            n *= -1.f;
        //_mirrorNormal.setValue(n);

        std::vector<tgt::vec3> vertices;

        tgt::vec3 temp = cam.getUpVector();
        if(abs(tgt::dot(temp, n) > 0.9f))
            temp = tgt::vec3(0.f, 1.f, 0.f);

        tgt::vec3 inPlaneA = tgt::normalize(tgt::cross(n, temp)) * 0.5f * _size.getValue();
        tgt::vec3 inPlaneB = tgt::normalize(tgt::cross(n, inPlaneA)) * 0.5f * _size.getValue();

        const tgt::vec3& base = _mirrorCenter.getValue();

        float angle = 2.f * tgt::PIf / static_cast<float>(_numVertices.getValue());
        for (int i = 0; i < _numVertices.getValue(); ++i) {
            tgt::vec3 dir = tgt::quat::rotate(inPlaneA, angle * static_cast<float>(i), n);
            vertices.push_back(tgt::vec3(base + dir));
        }

        FaceGeometry* mirror = new FaceGeometry(vertices);
        data.addData(_mirrorID.getValue(), mirror);
        _mirrorID.issueWrite();

        _invalidationLevel.setValid();
    }

}
