#ifndef SLICEEXTRACTOR_H__
#define SLICEEXTRACTOR_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/genericproperty.h"

namespace TUMVis {
    /**
     * Extracts a slice from a 3D image and renders it to a 2D texture.
     */
    class SliceExtractor : public AbstractProcessor {
    public:
        /**
         * Constructs a new SliceExtractor Processor
         **/
        SliceExtractor();

        /**
         * Destructor
         **/
        virtual ~SliceExtractor();

        virtual void process(DataContainer& data);

    private:

        GenericProperty<std::string> _sourceImageID;    ///< image ID for input image
        GenericProperty<std::string> _targetImageID;    ///< image ID for output image

        GenericProperty<size_t> _sliceNumber;           ///< number of the slice to extract

        static const std::string loggerCat_;
    };

}

#endif // SLICEEXTRACTOR_H__
