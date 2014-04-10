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

#ifndef LHHISTOGRAM_H__
#define LHHISTOGRAM_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/cameraproperty.h"

namespace campvis {
    /**
     * Creates Lookup volumes vor generation LH-Histograms of volumes as well as the LH histogram.
     */
    class LHHistogram : public AbstractProcessor {
    public:
        /**
         * Constructs a new LHHistogram Processor
         **/
        LHHistogram();

        /**
         * Destructor
         **/
        virtual ~LHHistogram();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "LHHistogram"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates Lookup volumes vor generation LH-Histograms of volumes as well as the LH histogram."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_intensitiesId;      ///< ID for input volume
        DataNameProperty p_gradientsId;   ///< ID for input gradient volume

        DataNameProperty p_outputFL;         ///< ID for output FL volume
        DataNameProperty p_outputFH;         ///< ID for output FH volume

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // LHHISTOGRAM_H__
