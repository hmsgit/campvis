// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef REFERENCECOUNTED_H__
#define REFERENCECOUNTED_H__

#include "tbb/include/tbb/atomic.h"

namespace campvis {
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