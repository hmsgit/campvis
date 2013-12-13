// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "tgt/logmanager.h"
#include "tgt/vector.h"
#include "tgt/event/eventlistener.h"

namespace campvis {
    class TransferFunctionProperty;

    /**
     * Event handler that maps mouse click-and-drag events to the windowing of a transfer function.
     * 
     */
    class TransFuncWindowingEventListener : public tgt::EventListener {
    public:
        /**
         * Creates a TransFuncWindowingEventListener.
         */
        explicit TransFuncWindowingEventListener(TransferFunctionProperty* property);

        /**
         * Virtual Destructor
         **/
        virtual ~TransFuncWindowingEventListener();

        /// \see tgt::EventListener::onEvent()
        virtual void onEvent(tgt::Event* e);

    protected:
        TransferFunctionProperty* _prop;

        bool _mousePressed;                 ///< Flag whether the mouse is currently pressed
        tgt::ivec2 _mouseDownPosition;      ///< Viewport coordinates where mouse button has been pressed
        tgt::vec2 _originalIntensityDomain; ///< TF intensity domain when mouse was pressed

        static const std::string loggerCat_;
    };

}

#endif // TRANSFUNCWINDOWINGEVENTHANDLER_H__
