#ifndef SLICEEXTRACTOR_H__
#define SLICEEXTRACTOR_H__

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
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class SliceExtractor : public VisualizationProcessor {
    public:
        /**
         * Constructs a new SliceExtractor Processor
         **/
        SliceExtractor(GenericProperty<tgt::ivec2>& canvasSize);

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

        virtual void process(DataContainer& data);

        GenericProperty<std::string> _sourceImageID;    ///< image ID for input image
        GenericProperty<std::string> _targetImageID;    ///< image ID for output image

        IntProperty _sliceNumber;                       ///< number of the slice to extract
        TransferFunctionProperty _transferFunction;     ///< Transfer function

    protected:
        void updateProperties(const ImageData* img);

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };

}

#endif // SLICEEXTRACTOR_H__
