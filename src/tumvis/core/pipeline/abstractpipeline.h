#ifndef ABSTRACTPIPELINE_H__
#define ABSTRACTPIPELINE_H__

#include "sigslot/sigslot.h"
#include "tgt/logmanager.h"
#include "tbb/include/tbb/spin_mutex.h"
#include "tbb/include/tbb/mutex.h"
#include "tbb/include/tbb/compat/condition_variable"
#include "core/tools/invalidationlevel.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/propertycollection.h"

#include <vector>

namespace TUMVis {
    class AbstractProcessor;
    class PipelineEvaluator;

    /**
     * Abstract base class for TUMVis Pipelines.
     * 
     */
    class AbstractPipeline : public HasPropertyCollection {
    friend class PipelineEvaluator;

    public:
        /**
         * Creates a AbstractPipeline.
         */
        AbstractPipeline();

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractPipeline();


        /**
         * Initializes this pipeline and all of its processors.
         * Everything that requires a valid OpenGL context or is otherwise expensive gets in here.
         * 
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Deinitializes this pipeline and all of its processors.
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void deinit();

        /**
         * Execute this pipeline.
         **/
        virtual void execute() = 0;

        /**
         * Returns the DataContainer of this pipeline
         * \return _data
         */
        const DataContainer& getDataContainer() const;

        /**
         * Returns the list of processors of this pipeline.
         * \return  _processors
         */
        const std::vector<AbstractProcessor*>& getProcessors() const;

        /**
         * Gets the name of this very pipeline. To be defined by every subclass.
         * \return  The name of this pipeline.
         */
        virtual const std::string getName() const = 0;
        
        /**
         * Gets the current InvalidationLevel of this pipeline.
         * \return _invalidationLevel
         */
        InvalidationLevel& getInvalidationLevel();

        /// Signal being emitted when this pipeline has been invalidated.
        sigslot::signal0<> s_PipelineInvalidated;

    protected:
        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * The default behaviour is just to set the invalidation level to invalid.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        /**
         * Slot getting called when one of the observed processors got invalidated.
         * The default behaviour is just to set the invalidation level to invalid.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorInvalidated(const AbstractProcessor* processor);

        /**
         * Executes the processor \a processor on the pipeline's data and locks its properties meanwhile.
         * \param   processor   Processor to execute.
         */
        void executeProcessor(AbstractProcessor& processor);
        DataContainer _data;                                ///< DataContainer containing local working set of data for this Pipeline

        std::vector<AbstractProcessor*> _processors;        ///< List of all processors of this pipeline
        InvalidationLevel _invalidationLevel;               ///< current invalidation level

        tbb::spin_mutex _localMutex;                        ///< mutex for altering local members
        tbb::mutex _evaluationMutex;                        ///< mutex for the evaluation of this pipeline

        static const std::string loggerCat_;
    };

}


#endif // ABSTRACTPIPELINE_H__
