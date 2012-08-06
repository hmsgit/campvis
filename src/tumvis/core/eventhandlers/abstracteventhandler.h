#ifndef ABSTRACTEVENTHANDLER_H__
#define ABSTRACTEVENTHANDLER_H__

#include "tgt/logmanager.h"
#include "tgt/event/event.h"

namespace TUMVis {

    /**
     * Abstract base class for TUMVis EventHandlers.
     * 
     */
    class AbstractEventHandler {
    public:
        /**
         * Creates a AbstractEventHandler.
         */
        AbstractEventHandler();

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractEventHandler();


        /**
         * Checks, whether the given event \a e is handled by this EventHandler.
         * \param e     The event to check
         * \return      True, if the given event is handled by this EventHandler.
         */
        virtual bool accept(tgt::Event* e) = 0;

        /**
         * Performs the event handling.
         * \param e     The event to handle
         */
        virtual void execute(tgt::Event* e) = 0;

    protected:

        static const std::string loggerCat_;
    };

}

#endif // ABSTRACTEVENTHANDLER_H__
