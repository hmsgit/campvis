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

namespace TUMVis {
    class ImageDataRenderTarget;

    /**
     * Abstract base class for TUMVis Pipelines.
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
         * Returns the PropertyCollection of this processor.
         * \return _properties
         */
        PropertyCollection& getPropertyCollection();

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
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * The default behaviour is to do nothing
         * \param poa   DataContainerObserverArgs   ObserverArgument struct containing the emitting DataContainer and hints to its changes
         */
        void onDataContainerDataAdded(const std::string& name, const DataHandle* dh);

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * If \a prop equals _renderTargetID, the s_renderTargetChanged is emitted.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        /// Signal emitted when the pipeline's render target has changed
        sigslot::signal0<> s_renderTargetChanged;

    protected:
        /**
         * Acquires and locks the OpenGL context, executes the processor \a processor on the pipeline's data 
         * and locks its properties meanwhile.
         * \param   processor   Processor to execute.
         */
        void lockGLContextAndExecuteProcessor(AbstractProcessor& processor);

        GenericProperty<tgt::ivec2> _renderTargetSize;      ///< Viewport size of target canvas
        StringProperty _renderTargetID;                     ///< ID of the render target image to be rendered to the canvas
        std::vector<AbstractEventHandler*> _eventHandlers;  ///< List of registered event handlers for the pipeline

        tgt::GLCanvas* _canvas;                             ///< Canvas hosting the OpenGL context for this pipeline.

        static const std::string loggerCat_;
    };

}

#endif // VISUALIZATIONPIPELINE_H__
