#ifndef VISUALIZATIONPIPELINE_H__
#define VISUALIZATIONPIPELINE_H__

#include "tgt/glcanvas.h"
#include "tgt/painter.h"
#include "tgt/shadermanager.h"
#include "tgt/event/eventlistener.h"
#include "core/eventhandlers/abstracteventhandler.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/properties/genericproperty.h"

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
        tgt::GLCanvas* _canvas;                             ///< Target canvas for rendering
        GenericProperty<tgt::ivec2> _canvasSize;            ///< Viewport size of target canvas
        GenericProperty<std::string> _renderTargetID;       ///< ID of the render target image to be rendered to the canvas
        tgt::Shader* _copyShader;                           ///< Shader for copying the render target to the framebuffer.

        std::vector<AbstractEventHandler*> _eventHandlers;  ///< List of registered event handlers for the pipeline

        static const std::string loggerCat_;
    };

}

#endif // VISUALIZATIONPIPELINE_H__
