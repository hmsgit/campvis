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

#ifndef ABSTRACTIMAGEFILTER_H__
#define ABSTRACTIMAGEFILTER_H__

#include <tbb/tbb.h>
#include "cgt/assert.h"
#include "modules/modulesapi.h"

#include <vector>

namespace campvis {
    class ImageRepresentationLocal;

    struct CAMPVIS_MODULES_API ImageFilterMedian {
    public:
        ImageFilterMedian(const ImageRepresentationLocal* input, ImageRepresentationLocal* output, size_t kernelSize);

        void operator() (const tbb::blocked_range<size_t>& range) const;

    protected:
        const ImageRepresentationLocal* _input;
        ImageRepresentationLocal* _output;
        size_t _kernelSize;
    };

}

#endif // ABSTRACTIMAGEFILTER_H__
