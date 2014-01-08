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

#include "proxygeometrygenerator.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {
    const std::string ProxyGeometryGenerator::loggerCat_ = "CAMPVis.modules.vis.ProxyGeometryGenerator";

    ProxyGeometryGenerator::ProxyGeometryGenerator()
        : AbstractProcessor()
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_geometryID("geometryID", "Output Geometry ID", "proxygeometry", DataNameProperty::WRITE)
        , p_clipX("clipX", "X Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
        , p_clipY("clipY", "Y Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
        , p_clipZ("clipZ", "Z Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
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

    void ProxyGeometryGenerator::updateResult(DataContainer& data) {
        ScopedTypedData<ImageData> img(data, p_sourceImageID.getValue());

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

                MeshGeometry* cube = GeometryDataFactory::createCube(volumeExtent, tgt::Bounds(texLLF, texURB));
                data.addData(p_geometryID.getValue(), cube);
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
