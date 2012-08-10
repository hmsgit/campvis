#ifndef CAMERAPROPERTY_H__
#define CAMERAPROPERTY_H__

#include "tgt/camera.h"
#include "core/properties/genericproperty.h"

namespace TUMVis {

    class CameraProperty : public GenericProperty<tgt::Camera> {
    public:
        /**
         * Creates a new CameraProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param cam       Camera to initialize the property with
         * \param il        Invalidation level that this property triggers
         */
        CameraProperty(const std::string& name, const std::string& title, tgt::Camera cam = tgt::Camera(), InvalidationLevel il = InvalidationLevel::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~CameraProperty();


    protected:

        static const std::string loggerCat_;
    };

}

#endif // CAMERAPROPERTY_H__
