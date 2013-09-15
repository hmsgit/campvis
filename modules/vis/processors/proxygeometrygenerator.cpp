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

#include "proxygeometrygenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {
    const std::string ProxyGeometryGenerator::loggerCat_ = "CAMPVis.modules.vis.ProxyGeometryGenerator";

    ProxyGeometryGenerator::ProxyGeometryGenerator()
        : AbstractProcessor()
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_geometryID("geometryID", "Output Geometry ID", "proxygeometry", DataNameProperty::WRITE)
        , p_clipX("clipX", "X Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(1))
        , p_clipY("clipY", "Y Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(1))
        , p_clipZ("clipZ", "Z Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(1))
	, _sourceTimestamp(0)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_geometryID);
        addProperty(&p_clipX);
        addProperty(&p_clipY);
        addProperty(&p_clipZ);
    }

    ProxyGeometryGenerator::~ProxyGeometryGenerator() {

    }

    void ProxyGeometryGenerator::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageData> img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                if (img.getDataHandle().getTimestamp() != _sourceTimestamp) {
                    p_clipX.setMaxValue(tgt::ivec2(static_cast<int>(img->getSize().x), static_cast<int>(img->getSize().x)));
                    p_clipY.setMaxValue(tgt::ivec2(static_cast<int>(img->getSize().y), static_cast<int>(img->getSize().y)));
                    p_clipZ.setMaxValue(tgt::ivec2(static_cast<int>(img->getSize().z), static_cast<int>(img->getSize().z)));

                    p_clipX.setValue(tgt::ivec2(0, static_cast<int>(img->getSize().x)));
                    p_clipY.setValue(tgt::ivec2(0, static_cast<int>(img->getSize().y)));
                    p_clipZ.setValue(tgt::ivec2(0, static_cast<int>(img->getSize().z)));
                    _sourceTimestamp = img.getDataHandle().getTimestamp();
                }
                tgt::Bounds volumeExtent = img->getWorldBounds(tgt::svec3(p_clipX.getValue().x, p_clipY.getValue().x, p_clipZ.getValue().x), tgt::svec3(p_clipX.getValue().y, p_clipY.getValue().y, p_clipZ.getValue().y));
                tgt::vec3 numSlices = tgt::vec3(img->getSize());


                tgt::vec3 texLLF(static_cast<float>(p_clipX.getValue().x), static_cast<float>(p_clipY.getValue().x), static_cast<float>(p_clipZ.getValue().x));
                texLLF /= numSlices;
                tgt::vec3 texURB(static_cast<float>(p_clipX.getValue().y), static_cast<float>(p_clipY.getValue().y), static_cast<float>(p_clipZ.getValue().y));
                texURB /= numSlices;

                MeshGeometry* cube = MeshGeometry::createCube(volumeExtent, tgt::Bounds(texLLF, texURB)).clone();
                data.addData(p_geometryID.getValue(), cube);
                p_geometryID.issueWrite();
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

}
