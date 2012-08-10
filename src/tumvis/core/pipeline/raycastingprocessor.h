#ifndef RAYCASTINGPROCESSOR_H__
#define RAYCASTINGPROCESSOR_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace TUMVis {
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
    class RaycastingProcessor : public VisualizationProcessor {
    public:
        /**
         * Creates a RaycastingProcessor.
         * \note    The render target size property of this RaycastingProcessor will automatically 
         *          be assigned as shared property of the given \a renderTargetSize property.
         * \see     VisualizationProcessor
         * \param   renderTargetSize            Reference to the parent pipeline's render target size property.
         * \param   fragmentShaderFileName      Filename for the fragment shader being automatically loaded.
         * \param   bindEntryExitDepthTextures  Flag whether to also bind the depth textures of the entry-/exit points.
         */
        RaycastingProcessor(GenericProperty<tgt::ivec2>& canvasSize, const std::string& fragmentShaderFileName, bool bindEntryExitDepthTextures);

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

        GenericProperty<std::string> _sourceImageID;    ///< image ID for input image
        GenericProperty<std::string> _entryImageID;     ///< image ID for output entry points image
        GenericProperty<std::string> _exitImageID;      ///< image ID for output exit points image

        TransferFunctionProperty _transferFunction;     ///< Transfer function
        FloatProperty _samplingStepSize;                ///< Ray casting step size
        BoolProperty _jitterEntryPoints;                ///< Flag whether to jitter the entry points

    protected:
        /**
         * Gets called by RaycastingProcessor::process().
         * Put additional (processor specific) setup code here, create and activate your render target(s), render
         * your quad and store your results in \a data.
         *
         * \sa      RaycastingProcessor::process()
         * \param   data    DataContainer to work on.
         */
        virtual void processImpl(DataContainer& data) = 0;

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
    };

}

#endif // RAYCASTINGPROCESSOR_H__
