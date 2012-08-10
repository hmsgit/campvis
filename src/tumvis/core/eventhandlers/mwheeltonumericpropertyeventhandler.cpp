// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
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

#include "mwheeltonumericpropertyeventhandler.h"
#include "tgt/assert.h"
#include "tgt/event/mouseevent.h"
#include "core/properties/numericproperty.h"

namespace TUMVis {
    const std::string MWheelToNumericPropertyEventHandler::loggerCat_ = "TUMVis.core.eventhandler.MWheelToNumericPropertyEventHandler";

    MWheelToNumericPropertyEventHandler::MWheelToNumericPropertyEventHandler(INumericProperty* property)
        : AbstractEventHandler()
        , _prop(property)
    {
        tgtAssert(_prop != 0, "Assigned property must not be 0.");
    }

    MWheelToNumericPropertyEventHandler::~MWheelToNumericPropertyEventHandler() {

    }

    bool MWheelToNumericPropertyEventHandler::accept(tgt::Event* e) {
        if (tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e)) {
            return me->action() == tgt::MouseEvent::WHEEL;
        }
        return false;
    }

    void MWheelToNumericPropertyEventHandler::execute(tgt::Event* e) {
        tgtAssert(dynamic_cast<tgt::MouseEvent*>(e) != 0, "Given event has wrong type. Check if the event is accepted by this event handler before executing it!");

        // this is only to be executed when the event was accepted, so the static cast is safe.
        tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
        tgtAssert(me->action() == tgt::MouseEvent::WHEEL, "Given event has wrong type. Check if the event is accepted by this event handler before executing it!");
        switch (me->button()) {
            case tgt::MouseEvent::MOUSE_WHEEL_UP:
                _prop->increment();
                e->ignore();
                break;
            case tgt::MouseEvent::MOUSE_WHEEL_DOWN:
                _prop->decrement();
                e->ignore();
                break;
        }
    }

}
