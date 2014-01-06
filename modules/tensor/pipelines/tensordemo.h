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

#ifndef TENSORDEMO_H__
#define TENSORDEMO_H__

#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/io/processors/mhdimagereader.h"
#include "modules/tensor/processors/tensoranalyzer.h"
#include "modules/vis/processors/sliceextractor.h"

namespace campvis {
    class TensorDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Small demo pipeline for tensor data visualization.
         */
        TensorDemo(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~TensorDemo();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractPipeline::getId()
        static const std::string getId() { return "TensorDemo"; };

    protected:
        MhdImageReader _imageReader;
        TensorAnalyzer _ta;
        SliceExtractor _sliceExtractor;

        MWheelToNumericPropertyEventListener _wheelHandler;
    };

}

#endif // TENSORDEMO_H__