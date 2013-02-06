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

#include "imagefilter.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"

#include "tbb/tbb.h"

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
        , p_sigma("Sigma", "Sigma", 1.f, .1f, 10.f)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_filterMode);
        addProperty(&p_kernelSize);
        addProperty(&p_sigma);
    }

    ImageFilter::~ImageFilter() {

    }

    void ImageFilter::process(DataContainer& data) {
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
            p_targetImageID.issueWrite();
        }
        else {
            LDEBUG("No suitable input image found.");
        }


        _invalidationLevel.setValid();
    }

}
