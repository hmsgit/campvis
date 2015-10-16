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

#include "confidencemapgenerator.h"

#include <tbb/tbb.h>
#include "cgt/logmanager.h"
#include "cgt/cgt_math.h"
#include "cgt/vector.h"
#include "core/tools/interval.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "modules/randomwalk/ext/RandomWalksLib/ConfidenceMaps2DFacade.h"

#include <algorithm>
#include <vector>

namespace campvis {

    static const GenericOption<std::string> solvers[4] = {
        GenericOption<std::string>("Eigen-LLT", "Eigen-LLT"),
        GenericOption<std::string>("Eigen-CG", "Eigen-CG"),
        GenericOption<std::string>("Eigen-BiCGSTAB", "Eigen-BiCGSTAB"),
        GenericOption<std::string>("Eigen-CG-Custom", "Eigen-CG-Custom"),
    };

    const std::string ConfidenceMapGenerator::loggerCat_ = "CAMPVis.modules.classification.ConfidenceMapGenerator";

    ConfidenceMapGenerator::ConfidenceMapGenerator()
        : AbstractProcessor()
        , p_sourceImageID("InputImage", "Input Image ID", "image", DataNameProperty::READ)
        , p_targetImageID("OutputConfidenceMap", "Output Confidence Map Image ID", "confidencemap", DataNameProperty::WRITE)
        , p_alpha("Alpha", "Alpha Parameter", 2.f, .1f, 10.f, 0.1f)
        , p_beta("Beta", "Beta Parameter", 100.f, 1.f, 1000.f, 0.1f)
        , p_gamma("Gamma", "Gamma Parameter", .06f, .01f, 1.f)
        , p_normalizeValues("NormalizeValues", "Normalize Values", false)
        , p_solver("FilterMode", "Filter Mode", solvers, 4)
        , p_numSteps("NumSteps", "Number of Solver Steps", 1000, 100, 5000)

        
    {
        addProperty(p_sourceImageID);
        addProperty(p_targetImageID);
        addProperty(p_alpha);
        addProperty(p_beta);
        addProperty(p_gamma);
        addProperty(p_normalizeValues);
        addProperty(p_solver);
        addProperty(p_numSteps);
    }

    ConfidenceMapGenerator::~ConfidenceMapGenerator() {

    }

    void ConfidenceMapGenerator::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());
        GenericImageRepresentationLocal<float, 1>::ScopedRepresentation previousResult(data, p_targetImageID.getValue());
        GenericImageRepresentationLocal<float, 1>::ScopedRepresentation velocities(data, p_targetImageID.getValue() + "velocities");

        if (input != 0 && input->getDimensionality() >= 2) {
            const cgt::svec3& imageSize = input->getSize();
            size_t numElements = input->getNumElements();
            size_t numElementsPerSlice = cgt::hmul(input->getSize().xy());
            float* outputValues = new float[numElements];

            // compute the Confidence Map
            tbb::parallel_for(tbb::blocked_range<size_t>(0, imageSize.z), [&] (const tbb::blocked_range<size_t>& range) {
                // Get each slice of the range through the confidence map generator
                ConfidenceMaps2DFacade _cmGenerator;
                _cmGenerator.setSolver(p_solver.getOptionValue(), p_numSteps.getValue());
                size_t offset = numElementsPerSlice * range.begin();

                std::vector<double> inputValues;
                const cgt::svec3& imageSize = input->getSize();
                inputValues.resize(numElementsPerSlice);

                for (size_t slice = range.begin(); slice < range.end(); ++slice) {
                    // Since the confidence map generator expects a vector of double, we need to copy the image data...
                    // meanwhile, we transform the pixel order to column-major:
                    for (size_t i = 0; i < numElementsPerSlice; ++i) {
                        size_t row = i / imageSize.y;
                        size_t column = imageSize.y - 1 - (i % imageSize.y);
                        size_t index = row + imageSize.x * column;
                        inputValues[i] = static_cast<double>(input->getElementNormalized(index + offset, 0));
                    }

                    // compute confidence map
                    _cmGenerator.setImage(inputValues, static_cast<int>(input->getSize().y), static_cast<int>(input->getSize().x), p_alpha.getValue(), p_normalizeValues.getValue());
                    std::vector<double> tmp = _cmGenerator.computeMap(p_beta.getValue(), p_gamma.getValue());

                    // copy and transpose back
                    for (size_t i = 0; i < numElementsPerSlice; ++i) {
                        size_t row = i / imageSize.y;
                        size_t column = imageSize.y - 1 - (i % imageSize.y);
                        size_t index = row + imageSize.x * column;
                        outputValues[index + offset] = static_cast<float>(tmp[i]);
                    }
                    offset += numElementsPerSlice;
                }
            });

            // perform alpha-beta filtering to avoid flickering:
            ImageData* output = new ImageData(input->getDimensionality(), cgt::svec3(input->getSize().x, input->getSize().y, 1), 1);
            auto outRep = GenericImageRepresentationLocal<float, 1>::create(output, outputValues);

            float dt = 0.5f;
            float a = 0.36f;
            float b = 0.005f;

            if (previousResult && velocities && previousResult->getNumElements() == outRep->getNumElements() && velocities->getNumElements() == outRep->getNumElements()) {
                // we have a previous result, so perform the filtering
                tbb::parallel_for(tbb::blocked_range<size_t>(0, outRep->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                    for (size_t i = range.begin(); i != range.end(); ++i) {
                        float xk = previousResult->getElement(i) + (velocities->getElement(i) * dt);
                        float vk = velocities->getElement(i);

                        float rk = outRep->getElement(i) - xk;

                        xk += a * rk;
                        vk += (b*rk) / dt;

                        outRep->setElement(i, xk);
                        const_cast<GenericImageRepresentationLocal<float, 1>*>(&*velocities)->setElement(i, vk);
                    }
                });
            }
            else {
                // we don't have a previous result, so initialize the filtering
                ImageData* velocityImage = new ImageData(input->getDimensionality(), cgt::svec3(input->getSize().x, input->getSize().y, 1), 1);
                auto veloRep = GenericImageRepresentationLocal<float, 1>::create(velocityImage, nullptr);

                tbb::parallel_for(tbb::blocked_range<size_t>(0, outRep->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                    for (size_t i = range.begin(); i != range.end(); ++i) {
                        float xk = 0.f;
                        float vk = 0.f;
                        float rk = outRep->getElement(i) - xk;

                        xk += a * rk;
                        vk += (b*rk) / dt;

                        veloRep->setElement(i, vk);
                    }
                });

                // save the velocity map for the filtering
                data.addData(p_targetImageID.getValue() + "velocities", velocityImage);
            }

            data.addData(p_targetImageID.getValue(), output);
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

}
