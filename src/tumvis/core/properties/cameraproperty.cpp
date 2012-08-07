#include "cameraproperty.h"

namespace TUMVis {

    const std::string CameraProperty::loggerCat_ = "TUMVis.core.datastructures.CameraProperty";

    CameraProperty::CameraProperty(const std::string& name, const std::string& title, tgt::Camera cam /*= tgt::Camera()*/, InvalidationLevel il /*= InvalidationLevel::INVALID_RESULT*/)
        : GenericProperty<tgt::Camera>(name, title, cam, il)
    {
    }

    CameraProperty::~CameraProperty() {
    }

}
