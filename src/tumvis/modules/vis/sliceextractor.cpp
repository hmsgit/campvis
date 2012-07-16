#include "sliceextractor.h"
#include "tgt/logmanager.h"
#include "core/datastructures/imagedata.h"

namespace TUMVis {

    const std::string SliceExtractor::loggerCat_ = "TUMVis.modules.vis.SliceExtractor";

    SliceExtractor::SliceExtractor()
        : AbstractProcessor()
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _targetImageID("targetImageID", "Output Image", "")
        , _sliceNumber("sliceNumber", "Slice Number", 0)
    {
        _properties.addProperty(&_sourceImageID);
        _properties.addProperty(&_targetImageID);
        _properties.addProperty(&_sliceNumber);
    }

    SliceExtractor::~SliceExtractor() {

    }

    void SliceExtractor::process(DataContainer& data) {
        const DataHandle* dh = data.getData(_sourceImageID.getValue());
        const ImageData* input;

        if (dh != 0 && (input = dynamic_cast<const ImageData*>(dh->getData())) != 0) {
            if (input->getDimensionality() == 3) {

            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }
    }

}
