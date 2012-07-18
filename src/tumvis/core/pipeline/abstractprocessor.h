#ifndef PROCESSOR_H__
#define PROCESSOR_H__

#include "tgt/logmanager.h"
#include "core/tools/invalidationlevel.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

#include <string>
#include <vector>

namespace TUMVis {
    class AbstractProcessor;

    /**
     * Observer Arguments for Property observers.
     */
    struct ProcessorObserverArgs : public GenericObserverArgs<AbstractProcessor> {
        /**
         * Creates new PropertyObserverArgs.
         * \param subject               Subject that emits the notification
         * \param invalidationLevel     Invalidation level of that property
         */
        ProcessorObserverArgs(const AbstractProcessor* subject, InvalidationLevel invalidationLevel)
            : GenericObserverArgs<AbstractProcessor>(subject)
            , _invalidationLevel(invalidationLevel)
        {}

        InvalidationLevel _invalidationLevel;       ///< Invalidation level of that processor
    };


    /**
     * Abstract base class for TUMVis Processors.
     * A processor implements a specific task, which it performs on the DataCollection passed
     * during process(). Properties provide a transparent layer for adjusting the processor's 
     * behaviour.
     * Once a processor has finished it sets it should set its invalidation level to valid. As
     * soon as one of its properties changes, the processor will be notified and possibliy
     * change its invalidation level. Observing pipelines will be notified of this and can
     * and have to decide which part of the pipeline has to be re-evaluated wrt. the processor's
     * invalidation level.
     * 
     * \sa AbstractPipeline
     */
    class AbstractProcessor : public GenericObserver<PropertyObserverArgs>, public GenericObservable<ProcessorObserverArgs> {

    /**
     * We have to find a trade-off:
     * On the one hand, we want to follow the information-hiding concept and keep the processor's
     * properties private. On the other hand pipelines will usually want direct access to them
     * (e.g. in order to setup data IDs or property sharing) and the properties in the PropertyCollection
     * are not strongly typed. Hence, we declare AbstractPipeline as our friend.
     */
    friend class AbstractPipeline;

    public:

        /**
         * Creates a AbstractProcessor.
         */
        AbstractProcessor();

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractProcessor();


        /**
         * Initializes the OpenGL context of the processor.
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Execute this processor.
         * \param data      DataContainer to work on.
         **/
        virtual void process(DataContainer& data) = 0;

        /**
         * Returns the invalidation level of this processor.
         * \return _invalidationLevel
         */
        const InvalidationLevel& getInvalidationLevel() const;

        /**
         * Returns the PropertyCollection of this processor.
         * \return  _properties
         */
        PropertyCollection& getPropertyCollection();

        /**
         * Update the processor's invalidation level by \a nl.
         * If \a nl is VALID, the processor's invalidation level will be set to VALID.
         * If \a nl is one of the INVALID_X state, the processor's corresponding flag will be set.
         * \param nl    Invalidation level to apply.
         */
        void applyInvalidationLevel(InvalidationLevel il);
        
        /**
         * Gets called when one of the observed properties changed notifies its observers.
         * \sa GenericObserver::onNotify, AbstractProperty
         * \param poa   PropertyObserverArgs    ObserverArgument struct containing the emitting property and its InvalidationLevel
         */
        virtual void onNotify(const PropertyObserverArgs& poa);


    protected:
        InvalidationLevel _invalidationLevel;       ///< current invalidation level of this processor

        PropertyCollection _properties;             ///< PropertyCollection of this processor


        static const std::string loggerCat_;
    };

}

#endif // PROCESSOR_H__
