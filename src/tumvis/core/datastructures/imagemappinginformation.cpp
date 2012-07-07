#include "imagemappinginformation.h"

namespace TUMVis {

    ImageMappingInformation::ImageMappingInformation() 
        : _realWorldValueMapping(LinearMaping<float>::identity)
        , _voxelToWorldTransformation(tgt::mat4::identity)
    {
    }

    const LinearMaping<float>& ImageMappingInformation::getRealWorldMapping() const {
        return _realWorldValueMapping;
    }

    void ImageMappingInformation::setRealWorldMapping(const LinearMaping<float>& rwvm) {
        _realWorldValueMapping = rwvm;
    }
}
