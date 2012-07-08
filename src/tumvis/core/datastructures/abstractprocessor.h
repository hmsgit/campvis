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

    protected:
        DataContainer _data;                    ///< DataContainer containing local working set of data for this Processor

        static const std::string loggerCat_;
    };

}

#endif // PROCESSOR_H__
