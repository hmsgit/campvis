#ifndef TRACKBALLNAVIGATIONEVENTHANDLER_H__
#define TRACKBALLNAVIGATIONEVENTHANDLER_H__

#include "tgt/logmanager.h"
#include "core/eventhandlers/abstracteventhandler.h"

namespace TUMVis {
    class CameraProperty;

    /**
     * Event handler that maps mouse wheel events to a numeric property.
     * 
     */
    class TrackballNavigationEventHandler : public AbstractEventHandler {
    public:
        /**
         * Creates a TrackballNavigationEventHandler.
         */
        TrackballNavigationEventHandler(CameraProperty* property);

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

    protected:
        CameraProperty* _cameraProperty;

        static const std::string loggerCat_;
    };

}

#endif // TRACKBALLNAVIGATIONEVENTHANDLER_H__
