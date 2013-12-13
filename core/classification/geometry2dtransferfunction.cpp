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

#include "geometry2dtransferfunction.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/classification/tfgeometry2d.h"

namespace campvis {

    const std::string Geometry2DTransferFunction::loggerCat_ = "CAMPVis.core.classification.Geometry2DTransferFunction";

    Geometry2DTransferFunction::Geometry2DTransferFunction(const tgt::svec2& size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/) 
        : GenericGeometryTransferFunction<TFGeometry2D>(tgt::svec3(size, 1), intensityDomain)
    {
    }

    Geometry2DTransferFunction::~Geometry2DTransferFunction() {
    }

    Geometry2DTransferFunction* Geometry2DTransferFunction::clone() const {
        Geometry2DTransferFunction* toReturn = new Geometry2DTransferFunction(_size.xy(), _intensityDomain);
        for (std::vector<TFGeometry2D*>::const_iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            toReturn->addGeometry((*it)->clone());
        }
        return toReturn;
    }

    size_t Geometry2DTransferFunction::getDimensionality() const {
        return 2;
    }

}