// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
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

#include "proxygeometrygenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {
    const std::string ProxyGeometryGenerator::loggerCat_ = "CAMPVis.modules.vis.ProxyGeometryGenerator";

    ProxyGeometryGenerator::ProxyGeometryGenerator()
        : AbstractProcessor()
        , _sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , _geometryID("geometryID", "Output Geometry ID", "proxygeometry", DataNameProperty::WRITE)
        , _clipX("clipX", "X Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
        , _clipY("clipY", "Y Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
        , _clipZ("clipZ", "Z Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
    {
        addProperty(&_sourceImageID);
        addProperty(&_geometryID);
        addProperty(&_clipX);
        addProperty(&_clipY);
        addProperty(&_clipZ);
    }

    ProxyGeometryGenerator::~ProxyGeometryGenerator() {

    }

    void ProxyGeometryGenerator::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageData> img(data, _sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                if (img.getDataHandle().getTimestamp() != _sourceTimestamp) {
                    _clipX.setMaxValue(tgt::ivec2(img->getSize().x, img->getSize().x));
                    _clipY.setMaxValue(tgt::ivec2(img->getSize().y, img->getSize().y));
                    _clipZ.setMaxValue(tgt::ivec2(img->getSize().z, img->getSize().z));

                    _clipX.setValue(tgt::ivec2(0, img->getSize().x));
                    _clipY.setValue(tgt::ivec2(0, img->getSize().y));
                    _clipZ.setValue(tgt::ivec2(0, img->getSize().z));
                    _sourceTimestamp = img.getDataHandle().getTimestamp();
                }
                tgt::Bounds volumeExtent = img->getWorldBounds(tgt::svec3(_clipX.getValue().x, _clipY.getValue().x, _clipZ.getValue().x), tgt::svec3(_clipX.getValue().y, _clipY.getValue().y, _clipZ.getValue().y));
                tgt::vec3 numSlices = tgt::vec3(img->getSize());


                tgt::vec3 texLLF(static_cast<float>(_clipX.getValue().x), static_cast<float>(_clipY.getValue().x), static_cast<float>(_clipZ.getValue().x));
                texLLF /= numSlices;
                tgt::vec3 texURB(static_cast<float>(_clipX.getValue().y), static_cast<float>(_clipY.getValue().y), static_cast<float>(_clipZ.getValue().y));
                texURB /= numSlices;

                MeshGeometry* cube = MeshGeometry::createCube(volumeExtent, tgt::Bounds(texLLF, texURB)).clone();
                data.addData(_geometryID.getValue(), cube);
                _geometryID.issueWrite();
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        _invalidationLevel.setValid();
    }

}
