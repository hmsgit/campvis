#ifndef TRACKBALLNAVIGATIONEVENTHANDLER_H__
#define TRACKBALLNAVIGATIONEVENTHANDLER_H__

#include "tgt/logmanager.h"
#include "tgt/navigation/trackball.h"
#include "core/eventhandlers/abstracteventhandler.h"

namespace TUMVis {
    class CameraProperty;

    class CamPropNavigationWrapper : public tgt::IHasCamera {
    public:
        CamPropNavigationWrapper(CameraProperty* camProp);
        virtual ~CamPropNavigationWrapper();

        virtual tgt::Camera* getCamera();

        virtual void update();

    private:
        CameraProperty* _cameraProperty;    ///< CameraProperty this class wraps around

        /// Temporary copy of the property's camera which will be modified and written back to the property upon update().
        tgt::Camera _localCopy;
        /// Flag whether _localCopy is dirty (needs to be written back)
        bool _dirty;
    };

    /**
     * EventHandler implementing a trackball navigation for a CameraProperty.
     * Implementation inspired by http://www.opengl.org/wiki/Trackball
     */
    class TrackballNavigationEventHandler : public AbstractEventHandler {
    public:
        /**
         * Creates a TrackballNavigationEventHandler.
         */
        TrackballNavigationEventHandler(CameraProperty* cameraProperty, const tgt::ivec2& viewportSize);

        /**
         * Virtual Destructor
         **/
        virtual ~TrackballNavigationEventHandler();


        /**
         * Checks, whether the given event \a e is handled by this EventHandler.
         * \param e     The event to check
         * \return      True, if the given event is handled by this EventHandler.
         */
        virtual bool accept(tgt::Event* e);

        /**
         * Performs the event handling.
         * \param e     The event to handle
         */
        virtual void execute(tgt::Event* e);

        void reinitializeCamera(const tgt::vec3& position, const tgt::vec3& focus, const tgt::vec3& upVector);

        void setCenter(const tgt::vec3& center);

    protected:
        CameraProperty* _cameraProperty;
        CamPropNavigationWrapper _cpnw;
        tgt::Trackball* _trackball;

        static const std::string loggerCat_;
    };

}

#endif // TRACKBALLNAVIGATIONEVENTHANDLER_H__
