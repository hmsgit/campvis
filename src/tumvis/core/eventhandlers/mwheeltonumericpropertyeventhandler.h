#ifndef MWHEELTONUMERICPROPERTYEVENTHANDLER_H__
#define MWHEELTONUMERICPROPERTYEVENTHANDLER_H__

#include "tgt/logmanager.h"
#include "core/eventhandlers/abstracteventhandler.h"
#include "core/properties/numericproperty.h"

namespace TUMVis {

    /**
     * Event handler that maps mouse wheel events to a numeric property.
     * 
     */
    class MWheelToNumericPropertyEventHandler : public AbstractEventHandler {
    public:
        /**
         * Creates a MWheelToNumericPropertyEventHandler.
         */
        MWheelToNumericPropertyEventHandler(INumericProperty* property);

        /**
         * Virtual Destructor
         **/
        virtual ~MWheelToNumericPropertyEventHandler();


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
        INumericProperty* _prop;

        static const std::string loggerCat_;
    };

}

#endif // MWHEELTONUMERICPROPERTYEVENTHANDLER_H__
