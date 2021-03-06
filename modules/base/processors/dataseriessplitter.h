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

#ifndef DATASERIESSPLITTER_H__
#define DATASERIESSPLITTER_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"

namespace campvis {
    /**
     * Extracts a single AbstractData instance from a DataSeries.
     */
    class DataSeriesSplitter : public AbstractProcessor {
    public:
        /**
         * Constructs a new DataSeriesSplitter Processor
         **/
        DataSeriesSplitter();

        /**
         * Destructor
         **/
        virtual ~DataSeriesSplitter();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "DataSeriesSplitter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts a single AbstractData instance from a DataSeries."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::STABLE; };

        DataNameProperty p_inputID;         ///< image ID for input data series
        DataNameProperty p_outputID;        ///< image ID for output data
        IntProperty p_imageIndex;           ///< index of the image to select

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dc);

        static const std::string loggerCat_;
    };

}

#endif // DATASERIESSPLITTER_H__
