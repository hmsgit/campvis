#ifndef VISUALIZATIONPIPELINE_H__
#define VISUALIZATIONPIPELINE_H__

#include "tgt/glcanvas.h"
#include "tgt/painter.h"
#include "tgt/event/eventlistener.h"
#include "core/eventhandlers/abstracteventhandler.h"
#include "core/pipeline/abstractpipeline.h"

#include <vector>

namespace TUMVis {

    /**
     * Abstract base class for TUMVis Pipelines.
     * 
     */
    class VisualizationPipeline : public AbstractPipeline, public tgt::EventListener, public tgt::Painter {
    public:
        /**
         * Creates a VisualizationPipeline.
         */
        VisualizationPipeline(tgt::GLCanvas* canvas = 0);

        /**
         * Virtual Destructor
         **/
        virtual ~VisualizationPipeline();


        /**
         * Initializes the OpenGL context of the pipeline and its processors.
         * \todo    merge with init() ?
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
         * Acquires the OpenGL context, and executes the pipeline
         */
        virtual void paint();

        /**
         * Is called when the viewport dimensions change, notifies the pipeline of that.
         * \param size  New viewport dimensions
         */
        virtual void sizeChanged(const tgt::ivec2& size);

        /**
         * Sets the target canvas for rendering.
         * \param canvas    Target canvas for rendering
         */
        void setCanvas(tgt::GLCanvas* canvas);

        /**
         * Returns the PropertyCollection of this processor.
         * \return _properties
         */
        PropertyCollection& getPropertyCollection();

    protected:
        DataContainer _data;                                ///< DataContainer containing local working set of data for this Pipeline
        tgt::GLCanvas* _canvas;                             ///< Target canvas for rendering
        tgt::ivec2 _canvasSize;                             ///< Viewport size of target canvas

        std::vector<AbstractEventHandler*> _eventHandlers;  ///< List of registered event handlers for the pipeline
        std::vector<AbstractProcessor*> _processors;        ///< List of all processors of this pipeline
        PropertyCollection _properties;                     ///< PropertyCollection of this pipeline, put in here all properties you want to be publicly accessible

        static const std::string loggerCat_;
    };

}

#endif // VISUALIZATIONPIPELINE_H__
