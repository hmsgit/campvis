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

#include "transfuncwindowingeventlistener.h"

#include "cgt/assert.h"
#include "cgt/event/mouseevent.h"
#include "core/classification/abstracttransferfunction.h"
#include "core/properties/transferfunctionproperty.h"

namespace campvis {
    const std::string TransFuncWindowingEventListener::loggerCat_ = "CAMPVis.core.eventhandler.TransFuncWindowingEventListener";

    TransFuncWindowingEventListener::TransFuncWindowingEventListener(TransferFunctionProperty* property)
        : cgt::EventListener()
        , _prop(property)
        , _mousePressed(false)
        , _mouseDownPosition(0, 0)
        , _originalIntensityDomain(0.f, 1.f)
    {
    }

    TransFuncWindowingEventListener::~TransFuncWindowingEventListener() {

    }

    void TransFuncWindowingEventListener::onEvent(cgt::Event* e) {
        if (_prop == nullptr)
            return;
        if (typeid(*e) != typeid(cgt::MouseEvent))
            return;

        // this is only to be executed when the event was accepted, so the static cast is safe.
        cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);
        if (me->action() == cgt::MouseEvent::PRESSED) {
            _mousePressed = true;
            _mouseDownPosition = cgt::ivec2(me->x(), me->y());
            _originalIntensityDomain = _prop->getTF()->getIntensityDomain();
            e->ignore();
        }
        else if (_mousePressed && me->action() == cgt::MouseEvent::RELEASED) {
            _mousePressed = false;
            e->ignore();
        }
        else if (_mousePressed && me->action() == cgt::MouseEvent::MOTION) {
            cgt::ivec2 currentPosition(me->x(), me->y());
            cgt::ivec2 delta = currentPosition - _mouseDownPosition;

            // apply shift
            float shift = static_cast<float>(delta.x) / 1000.f;
            cgt::vec2 newIntesityDomain = _originalIntensityDomain + shift;

            // compute and apply scaling
            float scale = delta.y < 0 ? 1.f + (delta.y / -10.f) : 1.f / (1.f + (delta.y / 10.f));
            float length = newIntesityDomain.y - newIntesityDomain.x;
            float offset = (scale - 1.f) * (length / 2.f);
            // clamp to avoid scaling to empty interval
            offset = cgt::clamp(offset, -length/2.f + 0.001f, length/2.f - 0.001f);

            newIntesityDomain.x -= offset;
            newIntesityDomain.y += offset;

            // triple-check for rock solid safety ;)
            if (newIntesityDomain.x > newIntesityDomain.y)
                std::swap(newIntesityDomain.x, newIntesityDomain.y);

            _prop->getTF()->setIntensityDomain(cgt::clamp(newIntesityDomain, cgt::vec2(0.f), cgt::vec2(1.f)));
            e->ignore();
        }
    }

    void TransFuncWindowingEventListener::setTransferFunctionProperty(TransferFunctionProperty* prop) {
        _prop = prop;
    }

}
