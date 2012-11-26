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

#ifndef TRANSFUNCWINDOWINGEVENTHANDLER_H__
#define TRANSFUNCWINDOWINGEVENTHANDLER_H__

#include "tgt/logmanager.h"
#include "tgt/vector.h"
#include "core/eventhandlers/abstracteventhandler.h"

namespace campvis {
    class TransferFunctionProperty;

    /**
     * Event handler that maps mouse click-and-drag events to the windowing of a transfer function.
     * 
     */
    class TransFuncWindowingEventHandler : public AbstractEventHandler {
    public:
        /**
         * Creates a TransFuncWindowingEventHandler.
         */
        TransFuncWindowingEventHandler(TransferFunctionProperty* property);

        /**
         * Virtual Destructor
         **/
        virtual ~TransFuncWindowingEventHandler();


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
        TransferFunctionProperty* _prop;

        bool _mousePressed;                 ///< Flag whether the mouse is currently pressed
        tgt::ivec2 _mouseDownPosition;      ///< Viewport coordinates where mouse button has been pressed
        tgt::vec2 _originalIntensityDomain; ///< TF intensity domain when mouse was pressed

        static const std::string loggerCat_;
    };

}

#endif // TRANSFUNCWINDOWINGEVENTHANDLER_H__
