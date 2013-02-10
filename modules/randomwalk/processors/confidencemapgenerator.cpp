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

#include "tbb/tbb.h"
#include "tgt/logmanager.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "modules/randomwalk/ext/RandomWalksLib/ConfidenceMaps2DFacade.h"

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
        CMGenerator(const ImageRepresentationLocal* input, float* output, const std::string& solver, float alpha, float beta, float gamma, bool normalizeValues)
            : _input(input)
            , _output(output)
            , _solver(solver)
            , _alpha(alpha)
            , _beta(beta)
            , _gamma(gamma)
            , _normalizeValues(normalizeValues)
        {
            tgtAssert(input != 0, "Pointer to Input must not be 0.");
            tgtAssert(output != 0, "Pointer to Output must not be 0.");
            _numElementsPerSlice = tgt::hmul(_input->getSize().xy());
        }


        void operator() (const tbb::blocked_range<size_t>& range) const {
            // Get each slice of the range through the confidence map generator
            ConfidenceMaps2DFacade _cmGenerator;
            _cmGenerator.setSolver(_solver);
            std::vector<double> inputValues;
            const tgt::svec3& imageSize = _input->getSize();
            inputValues.resize(_numElementsPerSlice);
            size_t offset = _numElementsPerSlice * range.begin();

            for (size_t slice = range.begin(); slice < range.end(); ++slice) {
                // Since the confidence map generator expects a vector of double, we need to copy the image data...
                // meanwhile, we transform the pixel order to column-major:
                for (size_t i = 0; i < _numElementsPerSlice; ++i) {
                    size_t row = i / imageSize.y;
                    size_t column = i % imageSize.y;
                    size_t index = row + imageSize.x * column;
                    inputValues[i] = static_cast<double>(_input->getElementNormalized(index + offset, 0));
                }

                // compute confidence map
                _cmGenerator.setImage(inputValues, _input->getSize().y, _input->getSize().x, _alpha, _normalizeValues);
                std::vector<double> tmp = _cmGenerator.computeMap(_beta, _gamma);

                // copy back
                for (size_t i = 0; i < _numElementsPerSlice; ++i) {
                    size_t row = i / imageSize.y;
                    size_t column = i % imageSize.y;
                    size_t index = row + imageSize.x * column;
                    _output[index + offset] = static_cast<float>(tmp[i]);
                }

                offset += _numElementsPerSlice;
            }
        }
    protected:
        const ImageRepresentationLocal* _input;
        float* _output;
        size_t _numElementsPerSlice;
        std::string _solver;
        float _alpha;
        float _beta;
        float _gamma;
        bool _normalizeValues;
    };

// ================================================================================================

    const std::string ConfidenceMapGenerator::loggerCat_ = "CAMPVis.modules.classification.ConfidenceMapGenerator";

    ConfidenceMapGenerator::ConfidenceMapGenerator()
        : AbstractProcessor()
        , p_sourceImageID("InputImage", "Input Image ID", "image", DataNameProperty::READ)
        , p_targetImageID("OutputConfidenceMap", "Output Confidence Map Image ID", "confidencemap", DataNameProperty::WRITE)
        , p_alpha("Alpha", "Alpha Parameter", 2.f, .1f, 10.f)
        , p_beta("Beta", "Beta Parameter", 100.f, 1.f, 1000.f)
        , p_gamma("Gamma", "Gamma Parameter", .06f, .01f, 1.f)
        , p_normalizeValues("NormalizeValues", "Noramlize Values", false)
        , p_solver("FilterMode", "Filter Mode", solvers, 4)
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

        if (input != 0 && input->getDimensionality() >= 2 && input->getParent()->getNumChannels() >= 1) {
            const tgt::svec3& imageSize = input->getSize();
            size_t numElements = input->getNumElements();
            float* outputValues = new float[numElements];

            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, imageSize.z),
                CMGenerator(input, outputValues, p_solver.getOptionValue(), p_alpha.getValue(), p_beta.getValue(), p_gamma.getValue(), p_normalizeValues.getValue()));

            ImageData* output = new ImageData(input->getDimensionality(), input->getSize(), 1);
            GenericImageRepresentationLocal<float, 1>* confidenceMap = GenericImageRepresentationLocal<float, 1>::create(output, outputValues);
            data.addData(p_targetImageID.getValue(), output);
            p_targetImageID.issueWrite();
        }
        else {
            LDEBUG("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

}
