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

#ifndef IMAGESERIESSPLITTER_H__
#define IMAGESERIESSPLITTER_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"

namespace campvis {
    /**
     * Splits an ImageSeries into its single images.
     */
    class ImageSeriesSplitter : public AbstractProcessor {
    public:
        /**
         * Constructs a new ImageSeriesSplitter Processor
         **/
        ImageSeriesSplitter();

        /**
         * Destructor
         **/
        virtual ~ImageSeriesSplitter();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ImageSeriesSplitter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Splits an ImageSeries into its single images."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_inputID;   ///< image ID for input image series
        DataNameProperty p_outputID;   ///< image ID for output image
        IntProperty p_imageIndex;           ///< index of the image to select

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dc);

        static const std::string loggerCat_;
    };

}

#endif // IMAGESERIESSPLITTER_H__
