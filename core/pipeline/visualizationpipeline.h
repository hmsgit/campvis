// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef VISUALIZATIONPIPELINE_H__
#define VISUALIZATIONPIPELINE_H__

#include "sigslot/sigslot.h"
#include "tgt/vector.h"
#include "tgt/event/eventlistener.h"
#include "core/eventhandlers/abstracteventhandler.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/properties/genericproperty.h"

#include <vector>

namespace tgt {
    class GLCanvas;
}

namespace campvis {
    class ImageDataRenderTarget;

    /**
     * Abstract base class for CAMPVis Pipelines.
     * 
     */
    class VisualizationPipeline : public AbstractPipeline, public tgt::EventListener {
    public:
        /**
         * Creates a VisualizationPipeline.
         */
        VisualizationPipeline();

        /**
         * Virtual Destructor
         **/
        virtual ~VisualizationPipeline();


        /**
         * Initializes the OpenGL context of the pipeline and its processors.
         * Must be called with a valid and locked OpenGL context.
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Deinitializes this pipeline and all of its processors.
         * Must be called with a valid and locked OpenGL context.
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void deinit();

        /**
         * Execute this pipeline.
         * Pipeline must have a valid canvas set before calling this method.
         **/
        virtual void execute() = 0;

        /**
         * Performs the event handling for the assigned canvas.
         * Default behaviour is to execute all assigned EventHandlers, may be overwritten by subclasses.
         * \param e     event parameters
         */
        virtual void onEvent(tgt::Event* e);

        /**
         * Sets the Canvas hosting the OpenGL context for this pipeline.
         * \param   canvas  Canvas hosting the OpenGL context for this pipeline
         */
        void setCanvas(tgt::GLCanvas* canvas);

        /**
         * Sets the size of the render target
         * \param size  New viewport dimensions
         */
        void setRenderTargetSize(const tgt::ivec2& size);

        /**
         * Returns the viewport size of the target canvas
         * \return _renderTargetSize
         */
        const tgt::ivec2& getRenderTargetSize() const;

        /**
         * Returns the ID of the render target image to be rendered to the canvas
         * \return  The DataHandle named _renderTargetID in the pipeline's DataContainer, 0 if no such handle exists.
         */
        const std::string& getRenderTargetID() const;

        /**
         * Adds the event handler \a eventHandler to this pipeline's list of event handlers.
         * \param   eventHandler    The event handler to add.
         */
        void addEventHandler(AbstractEventHandler* eventHandler);

        /// Signal emitted when the pipeline's render target has changed
        sigslot::signal0<> s_renderTargetChanged;

    protected:
        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, the s_renderTargetChanged signal will be emitted.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        void onDataContainerDataAdded(const std::string& name, const DataHandle& dh);

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * If \a prop equals _renderTargetID, the s_renderTargetChanged is emitted.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        /**
         * Acquires and locks the OpenGL context, executes the processor \a processor on the pipeline's data 
         * and locks its properties meanwhile.
         * \param   processor   Processor to execute.
         */
        void lockGLContextAndExecuteProcessor(AbstractProcessor* processor);

        GenericProperty<tgt::ivec2> _renderTargetSize;      ///< Viewport size of target canvas
        StringProperty _renderTargetID;                     ///< ID of the render target image to be rendered to the canvas
        std::vector<AbstractEventHandler*> _eventHandlers;  ///< List of registered event handlers for the pipeline

        tgt::GLCanvas* _canvas;                             ///< Canvas hosting the OpenGL context for this pipeline.

        static const std::string loggerCat_;
    };

}

#endif // VISUALIZATIONPIPELINE_H__
