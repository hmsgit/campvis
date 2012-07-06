#include "imagedata.h"

namespace TUMVis {
    const std::string ImageData::loggerCat_ = "TUMVis.core.datastructures.ImageData";

    ImageData::ImageData(size_t dimensionality, const tgt::svec3& size) 
        : _dimensionality(dimensionality)
        , _size(size)
    {
    }

    ImageData::~ImageData() {
    }

    size_t ImageData::getDimensionality() const {
        return _dimensionality;
    }

    const tgt::svec3& ImageData::getSize() const {
        return _size;
    }

    const ImageMappingInformation& ImageData::getMappingInformation() const {
        return _mappingInformation;
    }

    ImageMappingInformation& ImageData::getMappingInformation() {
        return _mappingInformation;
    }
}