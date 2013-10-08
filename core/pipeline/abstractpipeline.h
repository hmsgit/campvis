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

#ifndef ABSTRACTPIPELINE_H__
#define ABSTRACTPIPELINE_H__

#include "sigslot/sigslot.h"

#include "tgt/logmanager.h"
#include "tgt/vector.h"
#include "tgt/event/eventhandler.h"
#include "tgt/event/eventlistener.h"

#include "tbb/spin_mutex.h"
#include "tbb/mutex.h"

#include "core/datastructures/datacontainer.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/propertycollection.h"

#include <vector>

namespace tgt {
    class GLCanvas;
}

namespace campvis {
    class AbstractProcessor;

    /**
     * Abstract base class for CAMPVis Pipelines.
     */
    class AbstractPipeline : public HasPropertyCollection, public tgt::EventHandler, public tgt::EventListener {
    public:
        /**
         * Creates a AbstractPipeline.
         * If you derive from AbstractPipeline, you will have to implement the pipeline evaluation
         * logic yourself. You might want to have a look at AutoEvaluationPipeline.
         */
        AbstractPipeline();

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
         * Locks all processors.
         */
        void lockAllProcessors();

        /**
         * Unlocks all processors.
         */
        void unlockAllProcessors();

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


        DataContainer _data;                                ///< DataContainer containing local working set of data for this Pipeline

        std::vector<AbstractProcessor*> _processors;        ///< List of all processors of this pipeline
        tbb::atomic<bool> _enabled;                         ///< flag whether this pipeline is currently enabled

        tgt::GLCanvas* _canvas;                             ///< Canvas hosting the OpenGL context for this pipeline.
        IVec2Property _canvasSize;                          ///< original canvas size
        bool _ignoreCanvasSizeUpdate;

        DataNameProperty _renderTargetID;                   ///< ID of the render target image to be rendered to the canvas

        static const std::string loggerCat_;

        
    private:
        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, the s_renderTargetChanged signal will be emitted.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        void onDataContainerDataAdded(const std::string& name, const DataHandle& dh);

    };

}


#endif // ABSTRACTPIPELINE_H__
