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

#ifndef SCANLINECONVERTER_H__
#define SCANLINECONVERTER_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/modulesapi.h"

namespace campvis {
    class ImageData;

    /**
     * Performs scanline conversion from a curvilinear (US) image in a rectilinear image into
     * a rectilinear image given the fan parameters.
     */
    class CAMPVIS_MODULES_API ScanlineConverter : public AbstractProcessor {
    public:
        /**
         * Constructs a new ScanlineConverter Processor
         **/
        ScanlineConverter();

        /**
         * Destructor
         **/
        virtual ~ScanlineConverter();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ScanlineConverter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs scanline conversion from a curvilinear (US) image in a rectilinear image into a rectilinear image given the fan parameters."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        /**
         * Generates all the lookup vertices for the target image in pixel coordinates of the input image.
         * \param   inputImage  Pointer to the input image (required for meta data)
         * \return  A vector of all lookup vertices in the input image.
         */
        std::vector<cgt::vec3> generateLookupVertices(const ImageData* inputImage) const;

        DataNameProperty p_sourceImageID;       ///< ID for input image
        DataNameProperty p_targetImageID;       ///< ID for output confidence map image

        IVec2Property p_targetSize;             ///< Target image size
        Vec2Property p_origin;                  ///< Origin of the fan in the input image
        Vec2Property p_angles;                  ///< Start and end angles of the fan
        Vec2Property p_lengths;                 ///< Start and end radius of the fan

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        static const std::string loggerCat_;
    };

}

#endif // SCANLINECONVERTER_H__
