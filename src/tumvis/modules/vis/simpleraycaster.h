#ifndef SIMPLERAYCASTER_H__
#define SIMPLERAYCASTER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace TUMVis {
    class ImageData;

    /**
     * Performs a simple volume ray casting.
     * \todo    OpenGL supports up to 4 bound FBO. We can use them to generate multiple images
     *          in a single pass, e.g. first hit point, normals, MIP, DVR.
     * \todo    Create some kind of RaycastingProcessor class to inherit from.
     */
    class SimpleRaycaster : public VisualizationProcessor {
    public:
        /**
         * Constructs a new SimpleRaycaster Processor
         **/
        SimpleRaycaster(GenericProperty<tgt::ivec2>& canvasSize);

        /**
         * Destructor
         **/
        virtual ~SimpleRaycaster();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "SimpleRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a simple volume ray casting."; };

        virtual void process(DataContainer& data);

        GenericProperty<std::string> _sourceImageID;    ///< image ID for input image
        GenericProperty<std::string> _entryImageID;     ///< image ID for output entry points image
        GenericProperty<std::string> _exitImageID;      ///< image ID for output exit points image
        GenericProperty<std::string> _targetImageID;    ///< image ID for output image

        TransferFunctionProperty _transferFunction;     ///< Transfer function
        FloatProperty _samplingStepSize;
        BoolProperty _jitterEntryPoints;

    protected:
        std::string generateHeader() const;

        tgt::Shader* _shader;                           ///< Shader for raycasting

        static const std::string loggerCat_;
    };

}

#endif // SIMPLERAYCASTER_H__
