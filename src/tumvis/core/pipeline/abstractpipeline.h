#ifndef ABSTRACTPIPELINE_H__
#define ABSTRACTPIPELINE_H__

#include "tgt/glcanvas.h"
#include "tgt/logmanager.h"
#include "tgt/event/eventlistener.h"
#include "core/datastructures/datacontainer.h"
#include "core/eventhandlers/abstracteventhandler.h"
#include "core/properties/propertycollection.h"

#include <vector>

namespace TUMVis {

    /**
     * Abstract base class for TUMVis Pipelines.
     * 
     */
    class AbstractPipeline : public tgt::EventListener {
    public:
        /**
         * Creates a AbstractPipeline.
         */
        AbstractPipeline(tgt::GLCanvas* canvas = 0);

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractPipeline();


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

        std::vector<AbstractEventHandler*> _eventHandlers;  ///< List of registered event handlers for the pipeline
        PropertyCollection _properties;                     ///< PropertyCollection of this pipeline

        static const std::string loggerCat_;
    };

}


#endif // ABSTRACTPIPELINE_H__
