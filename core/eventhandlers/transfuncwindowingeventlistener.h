// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#ifndef TRANSFUNCWINDOWINGEVENTHANDLER_H__
#define TRANSFUNCWINDOWINGEVENTHANDLER_H__

#include "cgt/logmanager.h"
#include "cgt/vector.h"
#include "cgt/event/eventlistener.h"

#include "core/coreapi.h"

namespace campvis {
    class TransferFunctionProperty;

    /**
     * Event handler that maps mouse click-and-drag events to the windowing of a transfer function.
     * 
     */
    class CAMPVIS_CORE_API TransFuncWindowingEventListener : public cgt::EventListener {
    public:
        /**
         * Creates a TransFuncWindowingEventListener.
         * \param   property    The target transfer function property, may be 0.
         */
        explicit TransFuncWindowingEventListener(TransferFunctionProperty* property);

        /**
         * Virtual Destructor
         **/
        virtual ~TransFuncWindowingEventListener();

        /// \see cgt::EventListener::onEvent()
        virtual void onEvent(cgt::Event* e);

        /**
         * Sets the target transfer function property.
         * \param   prop    The new target transfer function property, may be 0.
         */
        void setTransferFunctionProperty(TransferFunctionProperty* prop);

    protected:
        TransferFunctionProperty* _prop;    ///< The target transfer function property, may be 0

        bool _mousePressed;                 ///< Flag whether the mouse is currently pressed
        cgt::ivec2 _mouseDownPosition;      ///< Viewport coordinates where mouse button has been pressed
        cgt::vec2 _originalIntensityDomain; ///< TF intensity domain when mouse was pressed

        static const std::string loggerCat_;
    };

}

#endif // TRANSFUNCWINDOWINGEVENTHANDLER_H__
