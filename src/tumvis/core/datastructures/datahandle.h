#ifndef datahandle_h__
#define datahandle_h__

#include "tbb/include/tbb/spin_mutex.h"
#include "tgt/logmanager.h"
#include "core/datastructures/abstractdata.h"

#include <string>
#include <set>

namespace TUMVis {

    class DataContainer;

    /**
     * A DataHandle is responsible to manage the lifetime of an AbstractData instance.
     * Therefore, it holds a pointer to the managed data object as well as a set of pointers to the
     * DataContainer instances containing this very DataHandle. Each DataContainer is responsible for
     * registering itsself as owner of its DataHandles (via DataHandle::addOwner()). When removing a
     * DataHandle from a container, make sure to deregister via DataHandle::removeOwner().
     *
     * \note    For clarity: A DataHandle can have multiple owners, as soon as the owner count drops
     *          to 0 it will be destroyed. Also remember that a DataHandle takes ownership of the 
     *          given AbstractData instance. So do not delete it once it has been assigned to a 
     *          DataHandle (respectively DataContainer).
     *
     * \todo    implement locks/mutexes as soon as we have a convenient library for that.
     */
    class DataHandle {
    
    // DataContainer is the only class allowed to access the private constructor and ownership modification methods.
    friend class DataContainer;

    public:
        /**
         * Destructor, will delete the managed AbstractData.
         */
        virtual ~DataHandle();

        /**
         * Grants const access to the managed AbstractData instance.
         * \return  _data;
         */
        const AbstractData* getData() const;

        /**
         * Grants access to the managed AbstractData instance.
         * \return  _data;
         */
        AbstractData* getData();


    private:
        /**
         * Creates a new DataHandle for \a data and takes its ownership in terms of lifetime management. 
         * The DataContainer \a owner will automatically be added to the set of owners of this DataHandle, so
         * it is NOT needed to call DataHandle::addOwner().
         *
         * \param   owner   Initial owner of this DataHandle (usually the DataContainer creating the handle)
         * \param   data    AbstractData instance to manage
         */
        DataHandle(const DataContainer* owner, AbstractData* data);

        /**
         * DO NOT USE - it is private on purpose!
         *
         * IMHO a DataHandle does not need a copy-constructor - in particular it could be a bad idea to use
         * one, because it does not exactly what you expect. If you really need a copy-constructor, please
         * make sure to implement it correctly.
         */
        DataHandle(const DataHandle& rhs);

        /**
         * DO NOT USE - it is private on purpose!
         *
         * IMHO a DataHandle does not need an assignment-operator - in particular it could be a bad idea to use
         * one, because it does not exactly what you expect. If you really need an assignment-operator, please
         * make sure to implement it correctly.
         */
        DataHandle& operator=(const DataHandle& rhs);

        /**
         * Registers \a owner as owner of the DataHandle \a handle.
         * \param   handle  DataHandle that gets another ownership.
         * \param   owner   Owner that shall be added to the owner list of \a handle.
         */
        static void addOwner(const DataHandle* handle, const DataContainer* owner);

        /**
         * Removes \a owner from the owner list of \a handle. If afterwards the number of owners is 0, \a handle will be deleted.
         * \param   handle  DataHandle of which to remove the ownership.
         * \param   owner   Owner that shall be removed from the owner list of \a handle.
         */
        static void removeOwner(const DataHandle* handle, const DataContainer* owner);


        AbstractData* const _data;                          ///< managed data
        mutable std::set<const DataContainer*> _owners;     ///< set of owners of this DataHandle
        mutable tbb::spin_mutex _localMutex;                ///< Mutex used when altering local members

        static const std::string loggerCat_;
    };

}

#endif // datahandle_h__