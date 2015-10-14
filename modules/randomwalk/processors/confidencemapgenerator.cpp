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

    class CMGenerator {
    public:
        CMGenerator(
            const ImageRepresentationLocal* input, float* output, const ConfidenceMapGenerator* processor)
            : _input(input)
            , _output(output)
            , _processor(processor)
        {
            cgtAssert(input != 0, "Pointer to Input must not be 0.");
            cgtAssert(output != 0, "Pointer to Output must not be 0.");
            _numElementsPerSlice = cgt::hmul(_input->getSize().xy());
            _fanAngles = Interval<float>(_processor->p_angles.getValue());
            _fanSize = Interval<float>(_processor->p_lengths.getValue());
        }


        void operator() (const tbb::blocked_range<size_t>& range) const {
            // Get each slice of the range through the confidence map generator
            ConfidenceMaps2DFacade _cmGenerator;
            _cmGenerator.setSolver(_processor->p_solver.getOptionValue(), _processor->p_numSteps.getValue());
            std::vector<double> inputValues;
            const cgt::svec3& imageSize = _input->getSize();
            inputValues.resize(_numElementsPerSlice);
            size_t offset = _numElementsPerSlice * range.begin();

            for (size_t slice = range.begin(); slice < range.end(); ++slice) {
                // Since the confidence map generator expects a vector of double, we need to copy the image data...
                if (! _processor->p_curvilinear.getValue()) {
                    // meanwhile, we transform the pixel order to column-major:
                    for (size_t i = 0; i < _numElementsPerSlice; ++i) {
                        size_t row = i / imageSize.y;
                        size_t column = i % imageSize.y;
                        size_t index = row + imageSize.x * column;
                        inputValues[i] = static_cast<double>(_input->getElementNormalized(index + offset, 0));
                    }
                }
                else {
                    // meanwhile, we transform the polar fan geometry to cartesian:
                    // x, y are the indices of the target (column-major) array
                    for (size_t y = 0; y < imageSize.x; ++y) {
                        float phi = _fanAngles.getLeft() + static_cast<float>(y) / static_cast<float>(imageSize.x) * _fanAngles.size();
                        for (size_t x = 0; x < imageSize.y; ++x) {
                            float r = _fanSize.getLeft() + static_cast<float>(x) / static_cast<float>(imageSize.y) * _fanSize.size();

                            cgt::vec3 cc(r * cos(phi) + _processor->p_origin.getValue().x, r * sin(phi) + _processor->p_origin.getValue().y, 0.f);
                            cgtAssert(x + imageSize.y * y < _numElementsPerSlice, "asdasd");
                            inputValues[x + imageSize.y * y] = static_cast<double>(_input->getElementNormalizedLinear(cc, 0));
                        }
                    }
                }

                // compute confidence map
                _cmGenerator.setImage(inputValues, static_cast<int>(_input->getSize().y), static_cast<int>(_input->getSize().x), _processor->p_alpha.getValue(), _processor->p_normalizeValues.getValue());
                std::vector<double> tmp = _cmGenerator.computeMap(_processor->p_beta.getValue(), _processor->p_gamma.getValue());

                // copy back
                if (! _processor->p_curvilinear.getValue()) {
                    for (size_t i = 0; i < _numElementsPerSlice; ++i) {
                        size_t row = i / imageSize.y;
                        size_t column = i % imageSize.y;
                        size_t index = row + imageSize.x * column;
                        _output[index + offset] = static_cast<float>(tmp[i]);
                    }
                }
                else {
                    // transform cartesian back to polar coordinates
                    // x, y are the indices of the target (row-major) array
                    for (size_t y = 0; y < imageSize.y; ++y) {
                        for (size_t x = 0; x < imageSize.x; ++x) {
                            float dx = static_cast<float>(x) - _processor->p_origin.getValue().x;
                            float dy = static_cast<float>(y) - _processor->p_origin.getValue().y;

                            float r = (sqrt(dx*dx + dy*dy) - _fanSize.getLeft()) / _fanSize.size();
                            float phi = atan2(dy, dx);
                            if (phi < 0.f)
                                phi += 2.f * cgt::PIf;

                            phi = (phi - _fanAngles.getLeft()) / _fanAngles.size();

                            // for now just nearest neighbour sampling:
                            int column = cgt::iround(r * imageSize.y);
                            int row = cgt::iround(phi * imageSize.x);
                            if (column > 0 && column < static_cast<int>(imageSize.y) && row > 0 && row < static_cast<int>(imageSize.x)) {
                                cgtAssert(x + imageSize.x * y < _numElementsPerSlice, "asdasd2");
                                cgtAssert(column + imageSize.y * row < _numElementsPerSlice, "asdasd3");
                                _output[x + imageSize.x * y] = tmp[column + imageSize.y * row];
                            }
                            else {
                                _output[x + imageSize.x * y] = 1.f;
                            }
                        }
                    }

                }
                offset += _numElementsPerSlice;
            }
        }
    protected:
        const ImageRepresentationLocal* _input;
        float* _output;
        size_t _numElementsPerSlice;
        const ConfidenceMapGenerator* _processor;

        Interval<float> _fanAngles;
        Interval<float> _fanSize;
    };

// ================================================================================================

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
        , p_curvilinear("Curvilinear", "Curvilinear Transducer?", false)
        , p_origin("PolarOrigin", "Polar Origin", cgt::vec2(0.f), cgt::vec2(-1000.f), cgt::vec2(1000.f), cgt::vec2(0.1f))
        , p_angles("PolarAngles", "Polar Angles", cgt::vec2(0.f, 1.f), cgt::vec2(0.f), cgt::vec2(1000.f), cgt::vec2(0.1f))
        , p_lengths("PolarLengths", "Polar Lengths", cgt::vec2(0.f, 100.f), cgt::vec2(0.f), cgt::vec2(1000.f), cgt::vec2(0.1f))
        
    {
        addProperty(p_sourceImageID);
        addProperty(p_targetImageID);
        addProperty(p_alpha);
        addProperty(p_beta);
        addProperty(p_gamma);
        addProperty(p_normalizeValues);
        addProperty(p_solver);
        addProperty(p_numSteps);
        addProperty(p_curvilinear);
        addProperty(p_origin);
        addProperty(p_angles);
        addProperty(p_lengths);
    }

    ConfidenceMapGenerator::~ConfidenceMapGenerator() {

    }

    void ConfidenceMapGenerator::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation input(data, p_sourceImageID.getValue());
        GenericImageRepresentationLocal<float, 1>::ScopedRepresentation previousResult(data, p_targetImageID.getValue());
        GenericImageRepresentationLocal<float, 1>::ScopedRepresentation velocities(data, p_targetImageID.getValue() + "velocities");

        if (input != 0 && input->getDimensionality() >= 2 && input->getParent()->getNumChannels() >= 1) {
            const cgt::svec3& imageSize = input->getSize();
            size_t numElements = input->getNumElements();
            float* outputValues = new float[numElements];

            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, imageSize.z),
                CMGenerator(input, outputValues, this));

            ImageData* output = new ImageData(input->getDimensionality(), cgt::svec3(input->getSize().x, input->getSize().y, 1), 1);
            auto outRep = GenericImageRepresentationLocal<float, 1>::create(output, outputValues);

            float dt = 0.5f;
            float a = 0.36f;
            float b = 0.005f;

            if (previousResult && velocities && previousResult->getNumElements() == outRep->getNumElements() && velocities->getNumElements() == outRep->getNumElements()) {
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

                data.addData(p_targetImageID.getValue() + "velocities", velocityImage);
            }

            data.addData(p_targetImageID.getValue(), output);
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

}
