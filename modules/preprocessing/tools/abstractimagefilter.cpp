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

#include "abstractimagefilter.h"


#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include <algorithm>

namespace campvis {

    ImageFilterMedian::ImageFilterMedian(const ImageRepresentationLocal* input, ImageRepresentationLocal* output, size_t kernelSize)
        : _input(input)
        , _output(output)
        , _kernelSize(kernelSize)
    {
        cgtAssert(input != 0, "Input image must not be 0.");
        cgtAssert(output != 0, "Output image must not be 0.");
        cgtAssert(kernelSize > 0, "Kernel Size must be greater 0.");
    }

    void ImageFilterMedian::operator()(const tbb::blocked_range<size_t>& range) const {
        size_t halfKernelDim = static_cast<size_t>(_kernelSize / 2);
        const cgt::svec3& size = _input->getSize();
        
        for (size_t index = range.begin(); index < range.end(); ++index) {
            cgt::svec3 position = _input->getParent()->indexToPosition(index);

            size_t zmin = position.z >= halfKernelDim ? position.z - halfKernelDim : 0;
            size_t zmax = std::min(position.z+halfKernelDim, size.z-1);
            size_t ymin = position.y >= halfKernelDim ? position.y - halfKernelDim : 0;
            size_t ymax = std::min(position.y+halfKernelDim, size.y-1);
            size_t xmin = position.x >= halfKernelDim ? position.x - halfKernelDim : 0;
            size_t xmax = std::min(position.x+halfKernelDim, size.x-1);

            cgt::svec3 npos;
            std::vector<float> values;
            for (npos.z=zmin; npos.z<=zmax; npos.z++) {
                for (npos.y=ymin; npos.y<=ymax; npos.y++) {
                    for (npos.x=xmin; npos.x<=xmax; npos.x++) {
                        values.push_back(_input->getElementNormalized(npos, 0));
                    }
                }
            }
            size_t medianPosition = values.size() / 2;
            std::nth_element(values.begin(), values.begin() + medianPosition, values.end());
            _output->setElementNormalized(index, 0, values[medianPosition]);
        }
    }

}
