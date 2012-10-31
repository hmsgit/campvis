// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef MWHEELTONUMERICPROPERTYEVENTHANDLER_H__
#define MWHEELTONUMERICPROPERTYEVENTHANDLER_H__

#include "tgt/logmanager.h"
#include "core/eventhandlers/abstracteventhandler.h"

namespace campvis {
    class INumericProperty;

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
