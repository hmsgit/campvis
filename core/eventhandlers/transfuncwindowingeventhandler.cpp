// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#include "transfuncwindowingeventhandler.h"

#include "tgt/assert.h"
#include "tgt/event/mouseevent.h"
#include "core/classification/abstracttransferfunction.h"
#include "core/properties/transferfunctionproperty.h"

namespace campvis {
    const std::string TransFuncWindowingEventHandler::loggerCat_ = "CAMPVis.core.eventhandler.TransFuncWindowingEventHandler";

    TransFuncWindowingEventHandler::TransFuncWindowingEventHandler(TransferFunctionProperty* property)
        : AbstractEventHandler()
        , _prop(property)
    {
        tgtAssert(_prop != 0, "Assigned property must not be 0.");
    }

    TransFuncWindowingEventHandler::~TransFuncWindowingEventHandler() {

    }

    bool TransFuncWindowingEventHandler::accept(tgt::Event* e) {
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
            if (me->action() == tgt::MouseEvent::PRESSED)
                return true;
            else if (_mousePressed && me->action() == tgt::MouseEvent::RELEASED)
                return true;
            else if (_mousePressed && me->action() == tgt::MouseEvent::MOTION)
                return true;
        }
        return false;
    }

    void TransFuncWindowingEventHandler::execute(tgt::Event* e) {
        tgtAssert(dynamic_cast<tgt::MouseEvent*>(e) != 0, "Given event has wrong type. Check if the event is accepted by this event handler before executing it!");

        // this is only to be executed when the event was accepted, so the static cast is safe.
        tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
        if (me->action() == tgt::MouseEvent::PRESSED) {
            _mousePressed = true;
            _mouseDownPosition = tgt::ivec2(me->x(), me->y());
            _originalIntensityDomain = _prop->getTF()->getIntensityDomain();
            e->ignore();
        }
        else if (_mousePressed && me->action() == tgt::MouseEvent::RELEASED) {
            _mousePressed = false;
            e->ignore();
        }
        else if (_mousePressed && me->action() == tgt::MouseEvent::MOTION) {
            tgt::ivec2 currentPosition(me->x(), me->y());
            tgt::ivec2 delta = currentPosition - _mouseDownPosition;

            // apply shift
            float shift = static_cast<float>(delta.x) / 1000.f;
            tgt::vec2 newIntesityDomain = _originalIntensityDomain + shift;

            // compute and apply scaling
            float scale = delta.y < 0 ? 1.f + (delta.y / -10.f) : 1.f / (1.f + (delta.y / 10.f));
            float length = newIntesityDomain.y - newIntesityDomain.x;
            float offset = (scale - 1.f) * (length / 2.f);
            // clamp to avoid scaling to empty interval
            offset = tgt::clamp(offset, -length/2.f + 0.001f, length/2.f - 0.001f);

            newIntesityDomain.x -= offset;
            newIntesityDomain.y += offset;

            _prop->getTF()->setIntensityDomain(tgt::clamp(newIntesityDomain, tgt::vec2(0.f), tgt::vec2(1.f)));
            e->ignore();
        }
    }

}
