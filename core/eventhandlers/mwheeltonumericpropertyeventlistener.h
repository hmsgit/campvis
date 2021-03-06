// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef MWHEELTONUMERICPROPERTYEVENTHANDLER_H__
#define MWHEELTONUMERICPROPERTYEVENTHANDLER_H__

#include "cgt/logmanager.h"
#include "cgt/event/eventlistener.h"

#include "core/coreapi.h"

namespace campvis {
    class INumericProperty;

    /**
     * Event handler that maps mouse wheel events to a numeric property.
     */
    class CAMPVIS_CORE_API MWheelToNumericPropertyEventListener : public cgt::EventListener {
    public:
        /**
         * Creates a MWheelToNumericPropertyEventListener.
         */
        explicit MWheelToNumericPropertyEventListener(INumericProperty* property);

        /**
         * Virtual Destructor
         **/
        virtual ~MWheelToNumericPropertyEventListener();

        /// \see cgt::EventListener::wheelEvent()
        virtual void wheelEvent(cgt::MouseEvent* e);

    protected:
        INumericProperty* _prop;        ///< The Property to map the event to

        static const std::string loggerCat_;
    };

}

#endif // MWHEELTONUMERICPROPERTYEVENTHANDLER_H__
