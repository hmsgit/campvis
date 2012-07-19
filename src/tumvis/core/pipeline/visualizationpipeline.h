#ifndef VISUALIZATIONPIPELINE_H__
#define VISUALIZATIONPIPELINE_H__

#include "sigslot/sigslot.h"
#include "tgt/glcanvas.h"
#include "tgt/painter.h"
#include "tgt/shadermanager.h"
#include "tgt/event/eventlistener.h"
#include "core/eventhandlers/abstracteventhandler.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/properties/genericproperty.h"

#include <vector>

namespace TUMVis {
    class ImageDataRenderTarget;

    /**
     * Abstract base class for TUMVis Pipelines.
     * 
     */
    class VisualizationPipeline : public AbstractPipeline, public tgt::EventListener, public sigslot::has_slots<> {
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
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Execute this pipeline.
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
         * Returns the DataHandle with the render target of this VisualizationPipeline in its current state.
         * 
         * \todo    This is not thread-safe, the object might be destroyed at any time. 
         *          Time for implementing reference counting?
         *          
         * \return  The DataHandle named _renderTargetID in the pipeline's DataContainer, 0 if no such handle exists.
         */
        const ImageDataRenderTarget* getRenderTarget() const;

        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * The default behaviour is to do nothing
         * \sa GenericObserver::onNotify, DataContainer
         * \param poa   DataContainerObserverArgs   ObserverArgument struct containing the emitting DataContainer and hints to its changes
         */
        void onDataContainerDataAdded(const std::string& name, const DataHandle* dh);


        sigslot::signal0<> s_renderTargetChanged;

    protected:
        GenericProperty<tgt::ivec2> _renderTargetSize;      ///< Viewport size of target canvas
        GenericProperty<std::string> _renderTargetID;       ///< ID of the render target image to be rendered to the canvas
        std::vector<AbstractEventHandler*> _eventHandlers;  ///< List of registered event handlers for the pipeline

        static const std::string loggerCat_;
    };

}

#endif // VISUALIZATIONPIPELINE_H__
