// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "imagerepresentationconverter.h"

#include <functional>

#include "core/properties/abstractproperty.h"
#include "core/gen_converterregistration.h"

namespace campvis {

    tbb::atomic<ImageRepresentationConverter*> ImageRepresentationConverter::_singleton;

    ImageRepresentationConverter& ImageRepresentationConverter::getRef() {
        if (_singleton == 0) {
            std::cout << "creating ImageRepresentationConverter...\n";
            ImageRepresentationConverter* tmp = new ImageRepresentationConverter();
            if (_singleton.compare_and_swap(tmp, 0) != 0) {
                delete tmp;
            }
        }

        return *_singleton;
    }

    void ImageRepresentationConverter::deinit() {
        delete _singleton;
        _singleton = nullptr;
    }

    size_t ImageRepresentationConverter::registerConversionFunction(const std::type_info& type, ConversionFunctionPointer funcPtr) {
        tbb::spin_mutex::scoped_lock lock(_mutex);
        std::type_index typeIndex(type);

        if (funcPtr != nullptr) {
            _conversionFunctionMap.insert(std::make_pair(typeIndex, funcPtr));
        }

        return _conversionFunctionMap.size();
    }

}