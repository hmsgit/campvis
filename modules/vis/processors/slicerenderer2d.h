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

#ifndef SLICERENDERER2D_H__
#define SLICERENDERER2D_H__

#include <string>

#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"
#include "tgt/event/eventlistener.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {

    /**
    * Applies a transfer function and other modifiers to a 2D input image and renders to screen buffer
    */
    class SliceRenderer2D : public VisualizationProcessor, public tgt::EventListener {
    public:
 
        /**
        * Constructs a new SliceRenderer2D Processor
        **/
        SliceRenderer2D(IVec2Property* viewportSizeProp);

        /**
        * Destructor
        **/
        virtual ~SliceRenderer2D();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "SliceRenderer2D"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Applies a transfer function and other modifiers to a 2D input image"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Jakob Weiss <weissj@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };


        DataNameProperty p_sourceImageID;                  ///< image ID for input image
        DataNameProperty p_targetImageID;                  ///< image ID for output image

        BoolProperty p_invertXAxis;                        ///< inverts the x axis (horizontal) of the image
        BoolProperty p_invertYAxis;                        ///< inverts the y axis (vertical) of the image

        IntProperty p_cropTop;                             ///< number of pixels to crop from the top
        IntProperty p_cropLeft;                            ///< number of pixels to crop from the left
        IntProperty p_cropBottom;                          ///< number of pixels to crop from the bottom
        IntProperty p_cropRight;                           ///< number of pixels to crop from the right

        TransferFunctionProperty p_transferFunction;     ///< Transfer function

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;

    private:
        tgt::svec3 _lastImgSize;
    };

}

#endif // SLICERENDERER2D_H__
