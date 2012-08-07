#include "imagemappinginformation.h"

namespace TUMVis {


    ImageMappingInformation::ImageMappingInformation(const tgt::vec3& offset, const tgt::vec3& voxelSize, const LinearMapping<float>& realWorldValueMapping /*= LinearMapping<float>::identity*/)
        : _offset(offset)
        , _voxelSize(voxelSize)
        , _realWorldValueMapping(realWorldValueMapping)
    {

    }


    const tgt::vec3& ImageMappingInformation::getOffset() const {
        return _offset;
    }

    const tgt::vec3& ImageMappingInformation::getVoxelSize() const {
        return _voxelSize;
    }

    const LinearMapping<float>& ImageMappingInformation::getRealWorldMapping() const {
        return _realWorldValueMapping;
    }

    void ImageMappingInformation::setRealWorldMapping(const LinearMapping<float>& rwvm) {
        _realWorldValueMapping = rwvm;
    }

}
