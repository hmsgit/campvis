#ifndef SLICEEXTRACTOR_H__
#define SLICEEXTRACTOR_H__

#include <string>

#include "tgt/shadermanager.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"

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


        virtual void init();

        virtual void process(DataContainer& data);

        GenericProperty<std::string> _sourceImageID;    ///< image ID for input image
        GenericProperty<std::string> _targetImageID;    ///< image ID for output image

        NumericProperty<size_t> _sliceNumber;           ///< number of the slice to extract

    protected:
        void updateProperties(const ImageData* img);

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };

}

#endif // SLICEEXTRACTOR_H__
