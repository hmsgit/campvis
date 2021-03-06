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

#ifndef ABSTRACTPIPELINE_H__
#define ABSTRACTPIPELINE_H__

#include "sigslot/sigslot.h"

#include "cgt/logmanager.h"
#include "cgt/runnable.h"
#include "cgt/vector.h"
#include "cgt/event/eventhandler.h"
#include "cgt/event/eventlistener.h"

#include <tbb/spin_mutex.h>
#include <tbb/mutex.h>

#include "core/coreapi.h"
#include "core/datastructures/datacontainer.h"
#include "core/pipeline/pipelinepainter.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/propertycollection.h"

#include <map>
#include <memory>
#include <vector>

namespace cgt {
    class GLCanvas;
}

namespace campvis {
    class AbstractProcessor;

    /**
     * Abstract base class for CAMPVis Pipelines.
     */
    class CAMPVIS_CORE_API AbstractPipeline : public HasPropertyCollection, public cgt::RunnableWithConditionalWait, public cgt::EventHandler, public cgt::EventListener {
    public:
        /**
         * Creates a AbstractPipeline.
         * If you derive from AbstractPipeline, you will have to implement the pipeline evaluation
         * logic yourself. You might want to have a look at AutoEvaluationPipeline.
         * 
         * \param   dataContainer   Reference to the DataContainer containing local working set of data
         *                          for this pipeline, must be valid the whole lifetime of this pipeline.
         */
        explicit AbstractPipeline(DataContainer& dataContainer);

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractPipeline();
        
        /**
         * Gets the name of this very pipeline. To be defined by every subclass.
         * \return  The name of this pipeline.
         */
        virtual std::string getName() const = 0;


        /**
         * Initializes this pipeline, its OpenGL context  and all of its processors.
         * Everything that requires a valid OpenGL context, valid construction of all other 
         * processors/properties or is otherwise expensive gets in here.
         * 
         * \note    Must be called with a valid and locked OpenGL context.
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Deinitializes this pipeline, its OpenGL context and all of its processors.
         * Everything that requires a valid OpenGL context, cannont be executed during destruction
         * or is otherwise expensive gets in here.
         * 
         * \note    Must be called with a valid and locked OpenGL context.
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void deinit();
        
        /**
         * Adds the processor \a processor to this pipeline's processor list.
         * \note    The s_invalidated signal of each processor on this list will be automatically 
         *          connected to onProcessorInvalidated() during initialization and disconnected
         *          during deinitialization.
         * \param   processor   The processor to add.
         */
        virtual void addProcessor(AbstractProcessor* processor);
                
        /**
         * Performs the event handling for the assigned canvas.
         * Default behaviour is to execute all assigned EventHandlers, may be overwritten by subclasses.
         * \param e     event parameters
         */
        virtual void onEvent(cgt::Event* e);

        /**
         * Entrance point for the pipeline thread.
         * \see Runnable::run
         */
        virtual void run();

        /**
         * Executes this pipeline.
         * To be implemented in the subclass.
         */
        virtual void executePipeline() = 0;
        
        /**
         * Sets the resultDirty flag of this pipeline and starts its execution if necessary.
         */
        void setPipelineDirty();

        /**
         * Paints an additional overlay directly onto the frame buffer.
         * Gets called from CampvisPainter just after copying the rendered image and just before
         * swapping the canvas' buffers. The default implementation performs nothing.
         */
        virtual void paint();

        /**
         * Returns the DataContainer of this pipeline, const version.
         * \return _dataContainer
         */
        const DataContainer& getDataContainer() const;

        /**
         * Returns the DataContainer of this pipeline, non-const version.
         * \return _dataContainer
         */
        DataContainer& getDataContainer();

        /**
         * Returns the list of processors of this pipeline.
         * \return  _processors
         */
        const std::vector<AbstractProcessor*>& getProcessors() const;

        /**
         * Returns the first processor of this pipeline whose name matches \a name.
         * \param   name    The name of the processor to find
         * \return  The first processor whose name matches \a name, 0 if no such processor exists.
         */
        AbstractProcessor* getProcessor(const std::string& name) const;

        /**
         * Returns the first processor of this pipeline whose name matches \a name.
         * \param   index    The index of the processor to get
         * \return  The first processor whose name matches \a name, 0 if no such processor exists.
         */
        AbstractProcessor* getProcessor(size_t index) const;

        /**
         * Gets the flag whether this pipeline is currently enabled.
         * \return _enabled
         */
        bool getEnabled() const;

        /**
         * Sets the flag whether this pipeline is currently enabled.
         * \param   enabled     New flag whether this pipeline is currently enabled.
         */
        void setEnabled(bool enabled);

        /**
         * Sets the Canvas hosting the OpenGL context for this pipeline.
         * \param   canvas  Canvas hosting the OpenGL context for this pipeline
         */
        void setCanvas(cgt::GLCanvas* canvas);

        /**
         * Sets the size of the render target
         * \param size  New viewport dimensions
         */
        void setRenderTargetSize(const cgt::ivec2& size);

        /**
         * Returns the viewport size of the target canvas
         * \return _canvasSize
         */
        const cgt::ivec2& getRenderTargetSize() const;

        /**
         * Returns the ID of the render target image to be rendered to the canvas
         * \return  The DataHandle named _renderTargetID in the pipeline's DataContainer, 0 if no such handle exists.
         */
        const std::string& getRenderTargetID() const;
        
        /**
         * Returns the canvas size 
         * \return  The IVec2Property with current canvas size on it
         */
        IVec2Property& getCanvasSize() { return _canvasSize; }

        /**
         * Returns this pipelines PipelinePainter.
         * \return  _painter
         */
        const std::unique_ptr<PipelinePainter>& getPipelinePainter() const;

        /// Signal emitted at the end of AbstractPipeline::init()
        sigslot::signal0 s_init;
        /// Signal emitted at the beginning of AbstractPipeline::deinit()
        sigslot::signal0 s_deinit;

    protected:
        /**
         * Forces the execution of the given processor regardless of its invalidation or enabled state.
         * \param   processor   Processor to execute.
         */
        void forceExecuteProcessor(AbstractProcessor* processor);

        /**
         * Executes the processor \a processor on the pipeline's data and locks its properties meanwhile.
         * \param   processor   Processor to execute.
         */
        void executeProcessor(AbstractProcessor* processor);
        
        /**
         * Executes \a processor and afterwards checks the OpenGL state to be valid.
         * \note    Only call from with a valid OpenGL context
         * \param   processor   Processor to execute
         */
        void executeProcessorAndCheckOpenGLState(AbstractProcessor* processor);

        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, setPipelineDirty will be called.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        virtual void onDataContainerDataAdded(std::string name, DataHandle dh);

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * The default behaviour is just to set the invalidation level to invalid.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);


        /// Pointer to the DataContainer containing local working set of data for this Pipeline, must not be 0.
        DataContainer* _dataContainer;

        std::vector<AbstractProcessor*> _processors;        ///< List of all processors of this pipeline

        cgt::GLCanvas* _canvas;                             ///< Canvas hosting the OpenGL context for this pipeline. We do *not* own this pointer.
        std::unique_ptr<PipelinePainter> _painter;          ///< PipelinePainter used to paint this pipeline's result onto the canvas.
        IVec2Property _canvasSize;                          ///< original canvas size
        bool _ignoreCanvasSizeUpdate;

        DataNameProperty _renderTargetID;                   ///< ID of the render target image to be rendered to the canvas

    private:
        tbb::atomic<bool> _enabled;                         ///< flag whether this pipeline is currently enabled
        tbb::atomic<bool> _pipelineDirty;                   ///< Flag whether this pipeline is dirty and executePipeline() needs to be called.

        static const std::string loggerCat_;

    };

}


#endif // ABSTRACTPIPELINE_H__
