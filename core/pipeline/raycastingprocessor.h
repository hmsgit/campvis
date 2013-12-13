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

#ifndef RAYCASTINGPROCESSOR_H__
#define RAYCASTINGPROCESSOR_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include "core/datastructures/imagerepresentationgl.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Base class for raycasting processors.
     * Offfers properties various common properties and automatic shader loading/linking.
     *
     * \note    The intended way to use this class is to inherit from it, add additional properties if necessary
     *          and implement the processImpl() method which will be called by RaycastingProcessor::process()
     *          after successful validation of the incoming images.
     *          Of course you can also directly overwrite process() yourself, but then you will need to to the
     *          sanity checks yourself.
     */
    class RaycastingProcessor : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /**
         * Creates a RaycastingProcessor.
         * \note    The render target size property of this RaycastingProcessor will automatically 
         *          be assigned as shared property of the given \a renderTargetSize property.
         * \see     VisualizationProcessor
         * \param   viewportSizeProp            Pointer to the parent pipeline's render target size property.
         * \param   fragmentShaderFileName      Filename for the fragment shader being automatically loaded.
         * \param   bindEntryExitDepthTextures  Flag whether to also bind the depth textures of the entry-/exit points.
         */
        RaycastingProcessor(IVec2Property* viewportSizeProp, const std::string& fragmentShaderFileName, bool bindEntryExitDepthTextures);

        /**
         * Destructor
         **/
        virtual ~RaycastingProcessor();

        /**
         * Initalizes the Processor, loads and builds the shader.
         * \see     AbstractProcessor::init
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Disposes the shader and deinitializes the processor.
         * \see     AbstractProcessor::deinit
         * \note    When overwriting this method, make sure to call the base class version at the end.
         */
        virtual void deinit();

        /**
         * Performs sanity checks, sets up the rendering and calls RaycastingProcessor::processImpl().
         * This method first reads the input image, entry and exit points from \a data and validates them. On sucess
         * the shader will be rebuild if necessary, the shader will be activated, common uniforms will be set and
         * the textures and transfer function will be bound before calling processImpl().
         *
         * \sa      RaycastingProcessor::processImpl()
         * \param   data    DataContainer to work on.
         */
        virtual void process(DataContainer& data);

        DataNameProperty p_sourceImageID;                ///< image ID for input image
        DataNameProperty p_entryImageID;                 ///< image ID for output entry points image
        DataNameProperty p_exitImageID;                  ///< image ID for output exit points image

        CameraProperty p_camera;                         ///< Camera used for ray casting
        TransferFunctionProperty p_transferFunction;     ///< Transfer function
        FloatProperty p_jitterStepSizeMultiplier;        ///< Step size multiplier for entry points jitter
        FloatProperty p_samplingRate;                    ///< Ray casting sampling rate

    protected:
        /**
         * Gets called by RaycastingProcessor::process().
         * Put additional (processor specific) setup code here, create and activate your render target(s), render
         * your quad and store your results in \a data.
         *
         * \sa      RaycastingProcessor::process()
         * \param   data    DataContainer to work on.
         * \param   image   The image to render
         */
        virtual void processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) = 0;

        /**
         * Returns an additional header that will be linked into the fragment shader.
         * Gets calles when building \a _shader. Overwrite this method to add a processor specific header to
         * your fragment shader.
         * \note    When overwriting this method make sure to call the base class version and append its result.
         * \return  The current default implementation returns an empty string.
         */
        virtual std::string generateHeader() const;

        const std::string _fragmentShaderFilename;      ///< Filename for the fragment shader being automatically loaded.
        tgt::Shader* _shader;                           ///< Shader for raycasting
        bool _bindEntryExitDepthTextures;               ///< Flag whether to also bind the depth textures of the entry-/exit points.

        static const std::string loggerCat_;

    private:
        clock_t _sourceImageTimestamp;
    };

}

#endif // RAYCASTINGPROCESSOR_H__
