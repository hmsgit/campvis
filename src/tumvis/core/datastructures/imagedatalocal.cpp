#include "imagedatalocal.h"


namespace TUMVis {

    const std::string ImageDataLocal::loggerCat_ = "TUMVis.core.datastructures.ImageDataLocal";

    ImageDataLocal::ImageDataLocal(size_t dimensionality, const tgt::svec3& size, WeaklyTypedPointer::BaseType baseType, size_t numChannels)
        : ImageData(dimensionality, size)
        , _baseType(baseType)
        , _numChannels(numChannels)
    {
    }

    ImageDataLocal::~ImageDataLocal() {
    }


}