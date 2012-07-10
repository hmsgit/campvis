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
     * 
     * \sa AbstractPipeline
     */
    class AbstractProcessor : GenericObserver<PropertyObserverArgs>, public GenericObservable<ProcessorObserverArgs> {
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
         * Execute this processor.
         **/
        virtual void process() = 0;

        /**
         * Adds the given DataHandle \a data, accessible by the key \name, to this DataContainer.
         * Already existing DataHandles with the same key will be removed from this DataContainer.
         *
         * \param   name    Key for accessing the DataHandle within this DataContainer
         * \param   data    DataHandle to add.
         **/
        virtual void addDataHandle(const std::string& name, const DataHandle* dh);

        /**
         * Returns the local DataContainer of this Processor.
         * \return _dataContainer
         **/
        const DataContainer& getDataContainer() const;

        /**
         * Returns the invalidation level of this processor.
         * \return _invalidationLevel
         */
        const InvalidationLevel& getInvalidationLevel() const;

        /**
         * Returns the PropertyCollection of this processor.
         * \return _properties
         */
        PropertyCollection& getPropertyCollection();

        /**
         * Update the processor's invalidation level by \a nl.
         * If \a nl is VALID, the processor's invalidation level will be set to VALID.
         * If \a nl is one of the INVALID_X state, the processor's corresponding flag will be set.
         * \param nl    Invalidation level to apply.
         */
        void applyInvalidationLevel(InvalidationLevel::NamedLevels nl);


        /**
         * Gets called when one of the observed properties changed notifies its observers.
         * \sa GenericObserver::onNotify, AbstractProperty
         * \param poa   PropertyObserverArgs    ObserverArgument struct containing the emitting property and its InvalidationLevel
         */
        virtual void onNotify(const PropertyObserverArgs& poa);


    protected:
        DataContainer _dataContainer;               ///< DataContainer containing local working set of data for this Processor
        InvalidationLevel _invalidationLevel;       ///< current invalidation level of this processor

        PropertyCollection _properties;             ///< PropertyCollection of this processor


        static const std::string loggerCat_;
    };

}

#endif // PROCESSOR_H__
