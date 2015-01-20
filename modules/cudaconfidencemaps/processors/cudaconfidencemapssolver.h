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

#ifndef CUDACONFIDENCEMAPSSOLVER_H__
#define CUDACONFIDENCEMAPSSOLVER_H__

#include <string>
#include <map>

#include <cgt/matrix.h>

#include <ext/threading.h>
#include <tbb/atomic.h>
#include <tbb/mutex.h>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

#include "core/datastructures/imagedata.h"

#include "modules/cudaconfidencemaps/core/cudaconfidencemaps_cuda.h"


namespace campvis {

    class CudaConfidenceMapsSolver : public AbstractProcessor {
    public:
        /**
         * Constructs a new CampcomMhdReceiver Processor
         **/
        CudaConfidenceMapsSolver();

        /**
         * Destructor
         **/
        virtual ~CudaConfidenceMapsSolver();

        /// \see AbstractProcessor::init()
        virtual void init();
        /// \see AbstractProcessor::deinit()
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "CudaConfidenceMapsSolver"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Solves the confidence maps problem iterativelly on the GPU"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Denis Declara <denis.declara@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
        /**
         * Updates the data container with the latest received transformation/position/image data
         * \param   dataContainer    DataContainer to work on
         */
        virtual void updateResult(DataContainer& dataContainer);

        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        int getActualConjugentGradientIterations() const;
        float getResidualNorm() const;

        DataNameProperty p_inputImage;              ///< ID for input volume
        DataNameProperty p_outputConfidenceMap;     ///< ID for output gradient volume

        ButtonProperty p_resetResult;

        BoolProperty  p_use8Neighbourhood;          ///< Wether to use 8- or 4-neighbourhood

        FloatProperty p_millisecondBudget;          ///< Maximum number of ms the solver can run

        FloatProperty p_gradientScaling;
        FloatProperty p_paramAlpha;
        FloatProperty p_paramBeta;
        FloatProperty p_paramGamma;

        BoolProperty  p_useAlphaBetaFilter;
        FloatProperty p_filterAlpha;                ///< Alpha parameter for Alpha-Beta Filter
        FloatProperty p_filterBeta;                 ///< Beta parameter for Alpha-Beta Filter
        
    protected:
        void resetSolutionVector();

        cuda::CudaConfidenceMapsSystemSolver _solver;
        
        static const std::string loggerCat_;
    };

}

#endif // CUDACONFIDENCEMAPSSOLVER_H__

