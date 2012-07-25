#ifndef DATACONTAINER_H__
#define DATACONTAINER_H__

#include "sigslot/sigslot.h"
#include "tbb/include/tbb/spin_mutex.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datahandle.h"
#include "core/tools/observer.h"

#include <string>
#include <map>

namespace TUMVis {
    /**
     * A DataContainer manages instances of AbstractData and offers access to them via string identifiers (names/keys).
     * Therefore, it stores them in DataHandles which take ownership of the AbstractData instance. Hence,
     * as soon as an AbstractData instance is added to a DataContainer via DataContainer::addData(), its 
     * lifetime is managed by the wrapping DataHandle instance and its reference counting mechanism.
     * Because the DataHandles are stored as const handles, the underlying data cannot be changed anymore. This
     * also ensures (hopefully) that nobody can do messy things, such as changing the data while some other 
     * thread is reading it. Theoretically this should be possible, but a correct implementation would require
     * some brain fuck.
     * 
     * \todo    Check thread-safety
     * 
     * \todo    If the original data changes, the other objects accessing that very DataHandle might want to 
     *          get notified of the change. Hence, some observer pattern might be useful.
     */
    class DataContainer {
    public:
        /**
         * Proxy class for scoped strongly-typed access to the data of a DataContainer.
         * From the outside ScopedTypedData<T> behaves exactly like a const T*, but internally it preserves the
         * reference counting of a DataHandle. Use this class when you want temporary access to a strongly-typed
         * data item in a DataContainer but don't want to to the dynamic_cast yourself.
         *
         * \tparam  T   Base class of the DataHandle data to test for
         */
        template<typename T>
        struct ScopedTypedData {
            /**
             * Creates a new DataHandle to the data item with the key \a name in \a dc, that behaves like a T*.
             * \param   dc      DataContainer to grab data from
             * \param   name    Key of the DataHandle to search for
             */
            ScopedTypedData(const DataContainer& dc, const std::string& name)
                : dh(dc.getData(name))
                , data(0)
            {
                if (dh != 0 && dh->getData() != 0) {
                    data = dynamic_cast<const T*>(dh->getData());
                }
            };

            /**
             * Destructor, deletes the internal DataHandle.
             */
            ~ScopedTypedData() {
                if (dh) {
                    delete dh;
                }
            };

            /**
             * Implicit conversion operator to const T*.
             * \return  The data in the DataHandle, may be 0 when no DataHandle was found, or the data is of the wrong type.
             */
            operator const T*() {
                return data;
            }

            /**
             * Implicit arrow operator to const T*.
             * \return  The data in the DataHandle, may be 0 when no DataHandle was found, or the data is of the wrong type.
             */
            const T* operator->() const {
                return data;
            }

        private:
            /// Not copy-constructable
            ScopedTypedData(const ScopedTypedData& rhs);
            /// Not assignable
            ScopedTypedData& operator=(const ScopedTypedData& rhs);

            const DataHandle* dh;   ///< DataHandle, may be 0
            const T* data;          ///< strongly-typed pointer to data, may be 0
        };


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
         * and will manage its lifetime. So don't even think about deleting \a data yourself!
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


        sigslot::signal2<const std::string&, const DataHandle*> s_dataAdded;

    private:
        std::map<std::string, const DataHandle*> _handles;
        mutable tbb::spin_mutex _localMutex;

        static const std::string loggerCat_;
    };

}

#endif // DATACONTAINER_H__
