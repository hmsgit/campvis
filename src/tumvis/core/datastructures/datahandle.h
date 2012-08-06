#ifndef datahandle_h__
#define datahandle_h__

namespace TUMVis {
    class AbstractData;

    /**
     * A DataHandle is responsible to manage the lifetime of an AbstractData instance.
     * Therefore, it implements a reference counting technique in cooperation with AbstractData.
     *
     * \note    For clarity: An AbstractData instance can be referenced by multiple DataHandles. As soon 
     *          as it is afterwards reference by 0 DataHandles the AbstractData instance will be destroyed.
     *          Also remember that a DataHandle takes ownership of the given AbstractData instance. So do
     *          not delete it once it has been assigned to a DataHandle (respectively DataContainer) or mess
     *          with its reference counting!
     * \note    Reference counting implementation inspired from Scott Meyers: More Effective C++, Item 29
     *
     * \todo    Check for thread-safety
     */
    class DataHandle {
    public:
        /**
         * Creates a new DataHandle for the given data.
         * \note    By passing the data to DataHandle you will transfer its ownership to the reference
         *          counting mechanism. Make sure not to interfere with it or delete \a data yourself!
         * \param   data    Data for the DataHandle
         */
        DataHandle(AbstractData* data);

        /**
         * Copy-constructor
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     Source DataHandle
         */
        DataHandle(const DataHandle& rhs);

        /**
         * Assignment operator
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     source DataHandle
         * \return  *this
         */
        DataHandle& operator=(const DataHandle& rhs);

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
         * \note    If the data is referenced by more than one object, this implies a copy of the data!
         * \return  A modifyable version of the held data.
         */
        AbstractData* getData();


    private:
        /**
         * Initializes the reference counting for the data.
         */
        void init();

        AbstractData* _data;                          ///< managed data
    };

}

#endif // datahandle_h__