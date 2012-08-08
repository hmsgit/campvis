#ifndef DRRRAYCASTER_H__
#define DRRRAYCASTER_H__

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
     * Creates a Digitally Reconstructed Radiograph.
     */
    class DRRRaycaster : public VisualizationProcessor {
    public:
        /**
         * Constructs a new DRRRaycaster Processor
         **/
        DRRRaycaster(GenericProperty<tgt::ivec2>& canvasSize);

        /**
         * Destructor
         **/
        virtual ~DRRRaycaster();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "DRRRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates a Digitally Reconstructed Radiograph."; };

        virtual void process(DataContainer& data);

        GenericProperty<std::string> _sourceImageID;    ///< image ID for input image
        GenericProperty<std::string> _entryImageID;     ///< image ID for output entry points image
        GenericProperty<std::string> _exitImageID;      ///< image ID for output exit points image
        GenericProperty<std::string> _targetImageID;    ///< image ID for output image

        TransferFunctionProperty _transferFunction;     ///< Transfer function
        FloatProperty _samplingStepSize;
        FloatProperty _shift;
        FloatProperty _scale;
        BoolProperty _invertMapping;

    protected:
        std::string generateHeader() const;

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };

}

#endif // DRRRAYCASTER_H__
