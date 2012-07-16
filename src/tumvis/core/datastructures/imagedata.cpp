#include "imagedata.h"

namespace TUMVis {
    const std::string ImageData::loggerCat_ = "TUMVis.core.datastructures.ImageData";

    ImageData::ImageData(size_t dimensionality, const tgt::svec3& size) 
        : AbstractData()
        , _dimensionality(dimensionality)
        , _size(size)
        , _numElements(tgt::hmul(size))
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

    size_t ImageData::getNumElements() const {
        return _numElements;
    }

    size_t ImageData::positionToIndex(const tgt::svec3& position) const {
        return position.x + (position.y * _size.x) + (position.z * _size.x * _size.y);
    }
}