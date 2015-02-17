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

namespace campvis {
    const std::string CudaConfidenceMapsSolver::loggerCat_ = "CAMPVis.modules.cudaconfidencemaps.solver";

    CudaConfidenceMapsSolver::CudaConfidenceMapsSolver() 
        : AbstractProcessor()
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputConfidenceMap("OutputConfidenceMap", "Output Confidence Map", "us.confidence", DataNameProperty::WRITE)
        , p_resetResult("ResetSolution", "Reset solution vector")
        , p_use8Neighbourhood("Use8Neighbourhood", "Use 8 Neighbourhood (otherwise 4)", true)
        , p_useFixedIterationCount("UseFixedIterationCount", "Use Fixed Iteration Count", false)
        , p_millisecondBudget("MillisecondBudget", "(P)CG Solver Time Budget", 25.0f, 1.0f, 1000.0f)
        , p_iterationBudget("IterationBudget", "(P)CG Solver Iteration Count", 100, 0, 1000)
        , p_gradientScaling("GradientScaling", "Scaling factor for gradients", 2.0f, 0.001f, 10.0f)
        , p_paramAlpha("Alpha", "Alpha (TGC)", 2.0f, 0.001f, 10.0f)
        , p_paramBeta("Beta", "Beta (Weight mapping)", 20.0f, 0.001f, 200.0f)
        , p_paramGamma("Gamma", "Gamma (Diagonal penalty)", 0.03f, 0.001f, 0.5f)
        , p_useAlphaBetaFilter("UseAlphaBetaFilter", "Use Alpha-Beta-Filter", true)
        , p_filterAlpha("FilterAlpha", "Filter Alpha", 0.36f, 0.0f, 1.0f)
        , p_filterBeta("FilterBeta", "Filter Beta", 0.005f, 0.0f, 1.0f)
        , _solver()
    {

        addProperty(p_inputImage);
        addProperty(p_outputConfidenceMap);

        addProperty(p_resetResult);
        addProperty(p_use8Neighbourhood);
        addProperty(p_useFixedIterationCount);
        addProperty(p_millisecondBudget);
        addProperty(p_iterationBudget);
        addProperty(p_gradientScaling);
        addProperty(p_paramAlpha);
        addProperty(p_paramBeta);
        addProperty(p_paramGamma);

        addProperty(p_useAlphaBetaFilter);
        addProperty(p_filterAlpha);
        addProperty(p_filterBeta);

        updatePropertyVisibility();
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
            bool use8Neighbourhood = p_use8Neighbourhood.getValue();
            float gradientScaling = p_gradientScaling.getValue();
            float alpha = p_paramAlpha.getValue();
            float beta = p_paramBeta.getValue();
            float gamma = p_paramGamma.getValue();

            // Setup the solver with the current Alpha-Beta-Filter settings
            _solver.enableAlphaBetaFilter(p_useAlphaBetaFilter.getValue());
            _solver.setAlphaBetaFilterParameters(p_filterAlpha.getValue(), p_filterBeta.getValue());

            cgt::ivec3 size = img->getSize();
            size_t elementCount = cgt::hmul(size);
            auto image = (unsigned char*)img->getWeaklyTypedPointer()._pointer;

            // Copy the image on the GPU and generate the equation system
            _solver.uploadImage(image, size.x, size.y, gradientScaling, alpha, beta, gamma, use8Neighbourhood);

            // Solve the equation system using Conjugate Gradient
            if (p_useFixedIterationCount.getValue()) {
                _solver.solveWithFixedIterationCount(p_iterationBudget.getValue());
            }
            else {
                _solver.solveWithFixedTimeBudget(p_millisecondBudget.getValue());
            }

            const float *solution = _solver.getSolution(size.x, size.y);

            // FIXME: Instead of copying the solution to a local representation first it would make
            // sense to directly create an opengl representation!
            ImageData *id = new ImageData(img->getParent()->getDimensionality(), size, img->getParent()->getNumChannels());
            cgt::Texture* resultTexture = new cgt::Texture(GL_TEXTURE_2D, size, GL_R32F, cgt::Texture::LINEAR);
            resultTexture->setWrapping(cgt::Texture::MIRRORED_REPEAT);
            resultTexture->uploadTexture(reinterpret_cast<const GLubyte*>(solution), GL_RED, GL_FLOAT);
            ImageRepresentationGL::create(id, resultTexture);
            id->setMappingInformation(img->getParent()->getMappingInformation());
            data.addData(p_outputConfidenceMap.getValue(), id);
        }
    }

    void CudaConfidenceMapsSolver::onPropertyChanged(const AbstractProperty* prop) {
        updatePropertyVisibility();
    }

    int CudaConfidenceMapsSolver::getActualConjugentGradientIterations() const {
        return _solver.getSolutionIterationCount();
    }

    float CudaConfidenceMapsSolver::getActualSolverExecutionTime() const {
        return _solver.getSystemSolveTime();
    }

    float CudaConfidenceMapsSolver::getResidualNorm() const {
        return _solver.getSolutionResidualNorm();
    }

    void CudaConfidenceMapsSolver::resetSolutionVector() {
        // Create a linear gradient image of the same size as the input image
        _solver.resetSolution();
    }

    void CudaConfidenceMapsSolver::updatePropertyVisibility() {
        // Hide properties that currently do not affect the processor
        bool useFixedIterationCount = p_useFixedIterationCount.getValue();
        p_millisecondBudget.setVisible(!useFixedIterationCount);
        p_iterationBudget.setVisible(useFixedIterationCount);
    }
}