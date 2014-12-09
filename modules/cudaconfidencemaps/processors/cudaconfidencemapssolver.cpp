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

#include "cudaconfidencemapssolver.h"

#include "core/datastructures/transformdata.h"
#include "core/datastructures/positiondata.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/tools/stringutils.h"

#include "modules/cudaconfidencemaps/core/cm_cusp_cuda_exports.h"

namespace campvis {
    const std::string CudaConfidenceMapsSolver::loggerCat_ = "CAMPVis.modules.cudaconfidencemaps.solver";

    CudaConfidenceMapsSolver::CudaConfidenceMapsSolver() 
        : AbstractProcessor()
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputConfidenceMap("OutputConfidenceMap", "Output Confidence Map", "us.confidence", DataNameProperty::WRITE)
        , p_resetResult("ResetSolution", "Reset solution vector")
        , p_iterations("IterationCount", "Conjugate Gradient Iterations", 200, 1, 10000)
        , p_gradientScaling("GradientScaling", "Scaling factor for gradients", 2.0f, 0.001, 10)
        , p_paramAlpha("Alpha", "Alpha (TGC)", 2.0f, 0.001, 10)
        , p_paramBeta("Beta", "Beta (Weight mapping)", 20.0f, 0.001, 200)
        , p_paramGamma("Gamma", "Gamma (Diagonal penalty)", 0.03f, 0.001, 0.5)
        , _solver(128, 128)
    {

        addProperty(p_inputImage);
        addProperty(p_outputConfidenceMap);

        addProperty(p_resetResult);
        addProperty(p_iterations);
        addProperty(p_gradientScaling);
        addProperty(p_paramAlpha);
        addProperty(p_paramBeta);
        addProperty(p_paramGamma);
    }

    CudaConfidenceMapsSolver::~CudaConfidenceMapsSolver() { }

    void CudaConfidenceMapsSolver::init() {
        p_resetResult.s_clicked.connect(this, &CudaConfidenceMapsSolver::resetSolutionVector);
        resetSolutionVector();
    }

    void CudaConfidenceMapsSolver::deinit() {
        p_resetResult.s_clicked.disconnect(this);
    }

    void CudaConfidenceMapsSolver::updateResult(DataContainer& data) {

        ImageRepresentationLocal::ScopedRepresentation img(data, p_inputImage.getValue());
        if (img != 0) {
            int iterations = p_iterations.getValue();
            float gradientScaling = p_gradientScaling.getValue();
            float alpha = p_paramAlpha.getValue();
            float beta = p_paramBeta.getValue();
            float gamma = p_paramGamma.getValue();
            bool isFlipped = true;

            cgt::ivec3 size = img->getSize();
            size_t elementCount = cgt::hmul(size);
            auto image = (unsigned char*)img->getWeaklyTypedPointer()._pointer;

            _solver.createSystem(image, size.x, size.y, gradientScaling, alpha, beta, gamma);

            _solver.solve(iterations);
            const float *solution = _solver.getSolution(size.x, size.y);

            float *solutionCopy = new float[elementCount];
            memcpy(solutionCopy, solution, sizeof(float) * elementCount);
            WeaklyTypedPointer wtpData(WeaklyTypedPointer::FLOAT, 1, solutionCopy);
            
            ImageData *id = new ImageData(img->getParent()->getDimensionality(), size, img->getParent()->getNumChannels());
            ImageRepresentationLocal::create(id, wtpData);
            id->setMappingInformation(img->getParent()->getMappingInformation());
            data.addData(p_outputConfidenceMap.getValue(), id);
        }
    }

    void CudaConfidenceMapsSolver::updateProperties(DataContainer& dataContainer) { }

    void CudaConfidenceMapsSolver::resetSolutionVector() {
        // Create a linear gradient image of the same size as the input image
        size_t elementCount = _solver.width * _solver.height;
        std::vector<float> gradient(elementCount);

        for(size_t i = 0; i < elementCount; ++i) {
            float value = static_cast<float>(i / _solver.width) / static_cast<float>(_solver.height-1);
            gradient[i] = value;
        }

        _solver.setInitialSolution(gradient);
    }
}