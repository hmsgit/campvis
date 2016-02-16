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

#include "flathierarchymapper.h"

#include "cgt/assert.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

namespace campvis {

    AbstractFlatHierarchyMapper::AbstractFlatHierarchyMapper(const ImageData* originalVolume)
        : _originalVolume(originalVolume)
        , _flatHierarchyTexture(nullptr)
        , _indexTexture(nullptr)
    {
        cgtAssert(originalVolume->getNumChannels() == 1, "FlatHierarchyMapper supports only single channel volumes!");

    }

    AbstractFlatHierarchyMapper::~AbstractFlatHierarchyMapper() {
        // currently disabled since we have the DataHandle hack:
        //delete _flatHierarchyTexture;
        
        //delete _indexTexture;
    }

    AbstractFlatHierarchyMapper* AbstractFlatHierarchyMapper::create(const ImageData* originalVolume) {
        if (originalVolume->getNumChannels() != 1) {
            LDEBUG("Cannot create a FlatHierarchyMapper for an image with more than 1 channel.");
            return nullptr;
        }

        const ImageRepresentationLocal* repLocal = originalVolume->getRepresentation<ImageRepresentationLocal>();
        if (dynamic_cast<const GenericImageRepresentationLocal<uint8_t, 1>*>(repLocal))
            return new FlatHierarchyMapper<uint8_t>(originalVolume);
        if (dynamic_cast<const GenericImageRepresentationLocal<int8_t, 1>*>(repLocal))
            return new FlatHierarchyMapper<int8_t>(originalVolume);
        if (dynamic_cast<const GenericImageRepresentationLocal<uint16_t, 1>*>(repLocal))
            return new FlatHierarchyMapper<uint16_t>(originalVolume);
        if (dynamic_cast<const GenericImageRepresentationLocal<int16_t, 1>*>(repLocal))
            return new FlatHierarchyMapper<int16_t>(originalVolume);
        if (dynamic_cast<const GenericImageRepresentationLocal<uint32_t, 1>*>(repLocal))
            return new FlatHierarchyMapper<uint32_t>(originalVolume);
        if (dynamic_cast<const GenericImageRepresentationLocal<int32_t, 1>*>(repLocal))
            return new FlatHierarchyMapper<int32_t>(originalVolume);
        if (dynamic_cast<const GenericImageRepresentationLocal<float, 1>*>(repLocal))
            return new FlatHierarchyMapper<float>(originalVolume);

        cgtAssert(false, "Should not reach this. If this assertion traps, something is wrong with the above code... :(");
        return nullptr;
    }

    cgt::Texture* AbstractFlatHierarchyMapper::getFlatHierarchyTexture() {
        return _flatHierarchyTexture;
    }

    cgt::Texture* AbstractFlatHierarchyMapper::getIndexTexture() {
        return _indexTexture;
    }

    std::string AbstractFlatHierarchyMapper::loggerCat_ = "CAMPVis.core.FlatHierarchyMapper";

}

