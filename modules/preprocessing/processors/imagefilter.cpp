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

#include "imagefilter.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include <tbb/tbb.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

namespace campvis {

    static const GenericOption<std::string> filterModes[2] = {
        GenericOption<std::string>("median", "Median"),
        GenericOption<std::string>("gauss", "Gauss"),
    };

    const std::string ImageFilter::loggerCat_ = "CAMPVis.modules.classification.ImageFilter";

    ImageFilter::ImageFilter()
        : AbstractProcessor()
        , p_sourceImageID("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_targetImageID("OutputGradients", "Output Gradient Volume ID", "gradients", DataNameProperty::WRITE)
        , p_filterMode("FilterMode", "Filter Mode", filterModes, 2)
        , p_kernelSize("KernelSize", "Kernel Size", 3, 3, 15)
        , p_sigma("Sigma", "Sigma", 1.f, .1f, 10.f, 0.1f)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_filterMode);
        addProperty(&p_kernelSize);
        addProperty(&p_sigma);
    }

    ImageFilter::~ImageFilter() {

    }

    void ImageFilter::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());

        if (input != 0 && input->getParent()->getNumChannels() == 1) {
            ImageData* id = new ImageData(input->getDimensionality(), input->getSize(), 1);
            ImageRepresentationLocal* output = input->clone(id);

            if (p_filterMode.getOptionValue() == "median") {
                tbb::parallel_for(
                    tbb::blocked_range<size_t>(0, input->getNumElements()), 
                    ImageFilterMedian(input, output, p_kernelSize.getValue()));
            }
            else if (p_filterMode.getOptionValue() == "gauss") {
                tbb::parallel_for(
                    tbb::blocked_range<size_t>(0, input->getNumElements()), 
                    ImageFilterGauss(input, output, p_kernelSize.getValue(), p_sigma.getValue()));
            }

            data.addData(p_targetImageID.getValue(), id);
        }
        else {
            LDEBUG("No suitable input image found.");
        }


        validate(INVALID_RESULT);
    }

}
