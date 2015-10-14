// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "geometry1dtransferfunction.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

#include "core/classification/tfgeometry1d.h"

namespace campvis {

    const std::string Geometry1DTransferFunction::loggerCat_ = "CAMPVis.core.classification.Geometry1DTransferFunction";

    Geometry1DTransferFunction::Geometry1DTransferFunction(size_t size, const cgt::vec2& intensityDomain /*= cgt::vec2(0.f, 1.f)*/) 
        : GenericGeometryTransferFunction<TFGeometry1D>(cgt::svec3(size, 1, 1), intensityDomain)
    {
    }

    Geometry1DTransferFunction::~Geometry1DTransferFunction() {
    }

    Geometry1DTransferFunction* Geometry1DTransferFunction::clone() const {
        Geometry1DTransferFunction* toReturn = new Geometry1DTransferFunction(_size.x, _intensityDomain);
        for (std::vector<TFGeometry1D*>::const_iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            toReturn->addGeometry((*it)->clone());
        }
        return toReturn;
    }

    size_t Geometry1DTransferFunction::getDimensionality() const {
        return 1;
    }

}