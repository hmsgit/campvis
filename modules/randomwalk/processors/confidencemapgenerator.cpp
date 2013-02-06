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

#include "confidencemapgenerator.h"

#include "tgt/logmanager.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "modules/randomwalk/ext/RandomWalksLib/ConfidenceMaps2DFacade.h"

#include <vector>

namespace campvis {

    static const GenericOption<std::string> filterModes[2] = {
        GenericOption<std::string>("median", "Median"),
        GenericOption<std::string>("gauss", "Gauss"),
    };

    const std::string ConfidenceMapGenerator::loggerCat_ = "CAMPVis.modules.classification.ConfidenceMapGenerator";

    ConfidenceMapGenerator::ConfidenceMapGenerator()
        : AbstractProcessor()
        , p_sourceImageID("InputImage", "Input Image ID", "image", DataNameProperty::READ)
        , p_targetImageID("OutputConfidenceMap", "Output Confidence Map Image ID", "confidencemap", DataNameProperty::WRITE)
        , p_alpha("Alpha", "Alpha Parameter", 2.f, .1f, 10.f)
        , p_beta("Beta", "Beta Parameter", 100.f, 1.f, 1000.f)
        , p_gamma("Gamma", "Gamma Parameter", .06f, .01f, 1.f)
        , p_normalizeValues("NormalizeValues", "Noramlize Values", false)
        , p_solver("FilterMode", "Filter Mode", filterModes, 2)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_alpha);
        addProperty(&p_beta);
        addProperty(&p_gamma);
        addProperty(&p_normalizeValues);
        addProperty(&p_solver);
    }

    ConfidenceMapGenerator::~ConfidenceMapGenerator() {

    }

    void ConfidenceMapGenerator::process(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());

        if (input != 0 && input->getDimensionality() >= 2 && input->getParent()->getNumChannels() == 1) {
            const tgt::svec3& imageSize = input->getSize();
            size_t numElements = input->getNumElements();
            size_t numElementsPerSlice = tgt::hmul(imageSize.xy());

            ConfidenceMaps2DFacade cmGenerator;
            std::vector<double> inputValues;
            inputValues.resize(numElementsPerSlice);
            float* outputValues = new float[numElements];

            // Get each slice through the confidence map generator
            size_t offset = 0;
            for (size_t slice = 0; slice < imageSize.z; ++slice) {
                // Since the confidence map generator expects a vector of double, we need to copy the image data...
                for (size_t i = 0; i < numElementsPerSlice; ++i) {
                    inputValues[i] = static_cast<double>(input->getElementNormalized(i + offset, 0));
                }

                // compute confidence map
                cmGenerator.setImage(inputValues, imageSize.x, imageSize.y, p_alpha.getValue(), p_normalizeValues.getValue());
                std::vector<double> tmp = cmGenerator.computeMap(p_beta.getValue(), p_gamma.getValue());
                
                // copy back
                for (size_t i = 0; i < numElementsPerSlice; ++i) {
                    outputValues[i + offset] = static_cast<float>(tmp[i]);
                }

                offset += numElementsPerSlice;
            }

            ImageData* output = new ImageData(input->getDimensionality(), input->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* confidenceMap = GenericImageRepresentationLocal<float, 1>::create(output, outputValues);
            data.addData(p_targetImageID.getValue(), output);
            p_targetImageID.issueWrite();
        }
        else {
            LDEBUG("No suitable input image found.");
        }

        _invalidationLevel.setValid();
    }

}
