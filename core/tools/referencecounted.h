// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef REFERENCECOUNTED_H__
#define REFERENCECOUNTED_H__

#include <tbb/atomic.h>

#include "core/coreapi.h"

namespace campvis {
    /**
     * Base class for reference counted objects.
     * 
     * \note    Implementation inspired from Scott Meyers: More Effective C++, Item 29
     * \todo    Check thread-safety
     */
    class CAMPVIS_CORE_API ReferenceCounted {
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

        static void deleteInstance(ReferenceCounted* instance);

    private:
        tbb::atomic<size_t> _refCount;      ///< number of references to this object
        bool _shareable;                    ///< flag whether this object is shareable, i.e. no non-const pointers to this object exist
    };
}


#endif // REFERENCECOUNTED_H__
