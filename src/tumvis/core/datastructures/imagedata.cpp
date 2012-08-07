#include "imagedata.h"

namespace TUMVis {
    const std::string ImageData::loggerCat_ = "TUMVis.core.datastructures.ImageData";

    ImageData::ImageData(size_t dimensionality, const tgt::svec3& size) 
        : AbstractData()
        , _dimensionality(dimensionality)
        , _size(size)
        , _numElements(tgt::hmul(size))
        , _mappingInformation(tgt::vec3(0.f), tgt::vec3(1.f)) // TODO: get offset/voxel size as parameter or put default values into ImageMappingInformation ctor.
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

    tgt::Bounds ImageData::getWorldBounds() const {
        return tgt::Bounds(_mappingInformation.getOffset(), _mappingInformation.getOffset() + (tgt::vec3(_size) * _mappingInformation.getVoxelSize()));
    }

    size_t ImageData::getNumElements() const {
        return _numElements;
    }

    size_t ImageData::positionToIndex(const tgt::svec3& position) const {
        return position.x + (position.y * _size.x) + (position.z * _size.x * _size.y);
    }
}