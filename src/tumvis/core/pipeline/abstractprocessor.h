#ifndef PROCESSOR_H__
#define PROCESSOR_H__

#include "tgt/logmanager.h"
#include "core/datastructures/datacontainer.h"

namespace TUMVis {

    /**
     * Abstract base class for TUMVis Processors.
     * 
     * \sa AbstractPipeline
     */
    class AbstractProcessor {
    public:
        enum InvalidationLevel {
            VALID               = 0,
            INVALID_RESULT      = 1 << 0,
            INVALID_SHADER      = 1 << 1,
        };

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
         * \returns Processor::_data
         **/
        const DataContainer& getDataContainer() const;

        /**
         * Returns the invalidation level of this processor.
         * Remind, that this is internally handled as a integer bit-set, so make sure to test via logic or.
         * \return Integer representation of _invalidationLevel
         */
        int getInvalidationLevel() const;

        /**
         * Update the processor's invalidation level by \a il.
         * If \a il is VALID, the processor's invalidation level will be set to VALID.
         * If \a il is one of the INVALID_X state, the processor's corresponding flag will be set.
         * \param il    Invalidation level to set.
         */
        void setInvalidationLevel(InvalidationLevel il);

    protected:
        DataContainer _data;                    ///< DataContainer containing local working set of data for this Processor
        int _invalidationLevel;                 ///< Invalidation level of this processor

        static const std::string loggerCat_;
    };

}

#endif // PROCESSOR_H__
