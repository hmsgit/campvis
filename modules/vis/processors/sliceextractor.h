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

#ifndef SLICEEXTRACTOR_H__
#define SLICEEXTRACTOR_H__

#include <string>

#include "tgt/buffer.h"
#include "tgt/vertexarrayobject.h"

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
    class FaceGeometry;
    class ImageData;

    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class SliceExtractor : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        enum SliceOrientation {
            XY_PLANE = 0,
            XZ_PLANE = 1,
            YZ_PLANE = 2
        };

        /**
         * Constructs a new SliceExtractor Processor
         **/
        SliceExtractor(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~SliceExtractor();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "SliceExtractor"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts a single slice from the input image and renders it using a transfer function."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_sourceImageID;                  ///< image ID for input image
        DataNameProperty p_targetImageID;                  ///< image ID for output image

        GenericOptionProperty<SliceOrientation> p_sliceOrientation; ///< orientation of the slice to extract
        IntProperty p_xSliceNumber;                         ///< number of the slice in X direction
        Vec4Property p_xSliceColor;                         ///< color for x marker
        IntProperty p_ySliceNumber;                         ///< number of the slice in Y direction
        Vec4Property p_ySliceColor;                         ///< color for y marker
        IntProperty p_zSliceNumber;                         ///< number of the slice in Z direction
        Vec4Property p_zSliceColor;                         ///< color for z marker
        TransferFunctionProperty p_transferFunction;     ///< Transfer function

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        void updateBorderGeometry();

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;

    private:
        clock_t _sourceImageTimestamp;
    };

}

#endif // SLICEEXTRACTOR_H__
