#ifndef DATACONTAINER_H__
#define DATACONTAINER_H__

#include "tbb/include/tbb/spin_mutex.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datahandle.h"

#include <string>
#include <map>

namespace TUMVis {

    /**
     * A DataContainer manages instances of AbstractData and offers access to them via string identifiers (names/keys).
     * Therefore, it stores them in DataHandles which take ownership of the AbstractData instance. Hence,
     * as soon as an AbstractData instance is added to a DataContainer via DataContainer::addData(), its 
     * lifetime is managed by the wrapping DataHandle instance.
     * Because the DataHandles are stored as const handles, the underlying data cannot be changed anymore. This
     * also ensures (hopefully) that nobody can do messy things, such as changing the data while some other 
     * thread is reading it or adding the same AbstractData instance twice to a DataContainer (which would 
     * really mess up the lifetime management!).
     * 
     * \todo    We definately want thread-safety here!
     * 
     * \todo    If the original data changes, the other objects accessing that very DataHandle might want to 
     *          get notified of the change. Hence, some observer pattern might be useful.
     */
    class DataContainer {
    public:
        /**
         * Creates a new empty DataContainer
         */
        DataContainer();

        /**
         * Destructor of the DataContainer. Will disconnect all DataHandles from this container.
         */
        ~DataContainer();


        /**
         * Adds the given AbstractData instance \a data, accessible by the key \name, to this DataContainer.
         * In doing so, the DataContainer (respectively the created DataHandle) takes ownership of \a data
         * and will manage its lifetime. So don't even dare to delete \a data yourself!
         *
         * \param   name    Key for accessing the DataHandle within this DataContainer
         * \param   data    DataHandle to add.
         * \return  The DataHandle which was created for \a data.
         */
        const DataHandle* addData(const std::string& name, AbstractData* data);

        /**
         * Adds the given DataHandle \a data, accessible by the key \name, to this DataContainer.
         * Already existing DataHandles with the same key will be removed from this DataContainer.
         *
         * \param   name    Key for accessing the DataHandle within this DataContainer
         * \param   data    DataHandle to add.
         */
        void addDataHandle(const std::string& name, const DataHandle* dh);

        /**
         * Checks whether this DataContainer contains a DataHandle with the given name.
         *
         * \param   name    Key of the DataHandle to search for
         * \return  true, if this DataContainer contains a DataHandle with the given name.
         */
        bool hasData(const std::string& name) const;

        /**
         * Returns the DataHandle with the given name from this container.
         * If no such DataHandle exists, this method returns 0.
         *
         * \param   name    Key of the DataHandle to search for
         * \return  The stored DataHandle with the given name, 0 if no such DataHandle exists.
         */
        const DataHandle* getData(const std::string& name) const;

        /**
         * Get the DataHandle with the given name from this container and tries to dynamic_cast it to const T*.
         * If no such DataHandle exists or the dynamic_cast fails, this method returns 0.
         *
         * \param   name    Key of the DataHandle to search for
         * \tparam  T       Target type of data for dynamic_cast.
         * \return  The stored DataHandle with the given name, casted to const T*, 0 if no such DataHandle exists or conversion failed.
         */
        template<typename T>
        inline const T* getTypedData(const std::string& name) const;

    private:
        std::map<std::string, const DataHandle*> _handles;
        mutable tbb::spin_mutex _localMutex;

        static const std::string loggerCat_;
    };

    template<typename T>
    const T* TUMVis::DataContainer::getTypedData(const std::string& name) const {
        const DataHandle* dh = getData(name);
        if (dh != 0)
            return dynamic_cast<const T*>(dh->getData());
        return 0;
    }

}

#endif // DATACONTAINER_H__
