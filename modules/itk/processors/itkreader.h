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

#ifndef ITKREADER_H_
#define ITKREADER_H_


#include "core/properties/allproperties.h"


namespace campvis {
    /**
     * Reads Image files using ITK
     * 
     * Should support all image types that are supported by the ITK library. Image Sequences are not supported.
     */
    class ItkReader : public AbstractProcessor {
    public:
        /**
         * Constructs a new ItkReader Processor
         **/
        ItkReader();

        /**
         * Destructor
         **/
        virtual ~ItkReader();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ItkReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads Image files using the ITK image reading facilities"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Jakob Weiss <weissj@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        StringProperty p_url;

        BoolProperty p_imageSeries;
        StringProperty p_lastUrl;

        DataNameProperty p_targetImageID;   ///< ID for output image

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& data);

        static const std::string loggerCat_;

    private:
        void ReadImageDirect(DataContainer& data);
        void ReadImageSeries(DataContainer& data);

        std::vector<std::string> GetImageFileNames();
    };

}

#endif // ITKREADER_H_
