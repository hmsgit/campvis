// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "tgt/logmanager.h"
#include "tgt/vector.h"
#include "tgt/event/eventhandler.h"
#include "tgt/event/eventlistener.h"

#include <tbb/spin_mutex.h>
#include <tbb/mutex.h>

#include "core/coreapi.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/propertycollection.h"

#include <map>
#include <vector>

namespace tgt {
    class GLCanvas;
}

namespace campvis {
    class AbstractProcessor;

    /**
     * Abstract base class for CAMPVis Pipelines.
     */
    class CAMPVIS_CORE_API AbstractPipeline : public HasPropertyCollection, public tgt::EventHandler, public tgt::EventListener {
    public:
        /**
         * Creates a AbstractPipeline.
         * If you derive from AbstractPipeline, you will have to implement the pipeline evaluation
         * logic yourself. You might want to have a look at AutoEvaluationPipeline.
         * 
         * \param   dc  Pointer to the DataContainer containing local working set of data for this 
         *              pipeline, must not be 0, must be valid the whole lifetime of this pipeline.
         */
        AbstractPipeline(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractPipeline();
        
        /**
         * Gets the name of this very pipeline. To be defined by every subclass.
         * \return  The name of this pipeline.
         */
        virtual const std::string getName() const = 0;


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
        virtual void onEvent(tgt::Event* e);


        /**
         * Returns the DataContainer of this pipeline, const version.
         * \return _data
         */
        const DataContainer& getDataContainer() const;

        /**
         * Returns the DataContainer of this pipeline, non-const version.
         * \return _data
         */
        DataContainer& getDataContainer();

        /**
         * Returns the list of processors of this pipeline.
         * \return  _processors
         */
        const std::vector<AbstractProcessor*>& getProcessors() const;

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
        void setCanvas(tgt::GLCanvas* canvas);

        /**
         * Sets the size of the render target
         * \param size  New viewport dimensions
         */
        void setRenderTargetSize(const tgt::ivec2& size);

        /**
         * Returns the viewport size of the target canvas
         * \return _canvasSize
         */
        const tgt::ivec2& getRenderTargetSize() const;

        /**
         * Returns the ID of the render target image to be rendered to the canvas
         * \return  The DataHandle named _renderTargetID in the pipeline's DataContainer, 0 if no such handle exists.
         */
        const std::string& getRenderTargetID() const;


        /// Signal emitted when the pipeline's render target has changed
        sigslot::signal0<> s_renderTargetChanged;

    protected:
        /**
         * Executes the processor \a processor on the pipeline's data and locks its properties meanwhile.
         * \param   processor   Processor to execute.
         * \param   unlockInExtraThred  If true, the call to processor->unlock() will be done in an extra thread.
         */
        void executeProcessor(AbstractProcessor* processor, bool unlockInExtraThred);
        
        /**
         * Acquires and locks the OpenGL context, executes the processor \a processor on the pipeline's data 
         * and locks its properties meanwhile.
         * \param   processor   Processor to execute.
         */
        void lockGLContextAndExecuteProcessor(AbstractProcessor* processor);
        
        /**
         * Executes \a processor and afterwards checks the OpenGL state to be valid.
         * \note    Only call from with a valid OpenGL context
         * \param   processor   Processor to execute
         */
        void executeProcessorAndCheckOpenGLState(AbstractProcessor* processor);

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * The default behaviour is just to set the invalidation level to invalid.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, the s_renderTargetChanged signal will be emitted.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        virtual void onDataContainerDataAdded(const std::string& name, const DataHandle& dh);

        /// Pointer to the DataContainer containing local working set of data for this Pipeline, must not be 0.
        DataContainer* _data;

        std::vector<AbstractProcessor*> _processors;        ///< List of all processors of this pipeline
        tbb::atomic<bool> _enabled;                         ///< flag whether this pipeline is currently enabled

        tgt::GLCanvas* _canvas;                             ///< Canvas hosting the OpenGL context for this pipeline.
        IVec2Property _canvasSize;                          ///< original canvas size
        bool _ignoreCanvasSizeUpdate;

        DataNameProperty _renderTargetID;                   ///< ID of the render target image to be rendered to the canvas

        static const std::string loggerCat_;

    };

}


#endif // ABSTRACTPIPELINE_H__
