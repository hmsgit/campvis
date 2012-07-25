#ifndef REFERENCECOUNTED_H__
#define REFERENCECOUNTED_H__

#include "tbb/include/tbb/atomic.h"

namespace TUMVis {
    /**
     * Base class for reference counted objects.
     * 
     * \note    Implementation inspired from Scott Meyers: More Effective C++, Item 29
     * \todo    Check thread-safety
     */
    class ReferenceCounted {
    public:
        /**
         * Create a new reference counted object.
         */
        ReferenceCounted();

        /**
         * Copy-constructor for reference counted objects.
         * \param   rhs source object
         */
        ReferenceCounted(const ReferenceCounted& rhs);

        /**
         * Assignment operator for reference counted objects
         * \param   rhs source object
         * \return  *this
         */
        ReferenceCounted& operator= (const ReferenceCounted& rhs);

        /**
         * Pure virtual destructor.
         */
        virtual ~ReferenceCounted() = 0;

        /**
         * Increment the reference count.
         */
        void addReference();

        /**
         * Decrement the reference count and delete the object if necessary.
         */
        void removeReference();

        /**
         * Mark this object as not shareable, i.e. there exist non-const pointers to this object.
         */
        void markUnsharable();

        /**
         * Returns, whether this object is shareable, i.e. no non-const pointers to this object exist.
         * \return  _shareable
         */
        bool isShareable() const;

        /**
         * Returns, whether this object has more than one references.
         * \return  _refCount > 1
         */
        bool isShared() const;

    private:
        tbb::atomic<size_t> _refCount;      ///< number of references to this object
        bool _shareable;                    ///< flag whether this object is shareable, i.e. no non-const pointers to this object exist
    };
}


#endif // REFERENCECOUNTED_H__
