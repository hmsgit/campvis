// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef QTQtDataHandle_H__
#define QTQtDataHandle_H__

#include "core/datastructures/datahandle.h"
#include <QMetaType>

namespace campvis {

    /**
     * Wraps around a DataHandle to make it suitable for QVariant and queued connections.
     */
    class QtDataHandle : public DataHandle {
    public:
        /**
         * Creates a new QtDataHandle for the given data.
         * \note    By passing the data to QtDataHandle you will transfer its ownership to the reference
         *          counting mechanism. Make sure not to interfere with it or delete \a data yourself!
         * \param   data    Data for the QtDataHandle
         */
        QtDataHandle(AbstractData* data = 0)
            : DataHandle(data)
        {}

        /**
         * Copy-constructor
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     Source QtDataHandle
         */
        QtDataHandle(const QtDataHandle& rhs)
            : DataHandle(rhs)
        {}

        /**
         * Copy-constructor
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     Source DataHandle
         */
        explicit QtDataHandle(const DataHandle& rhs)
            : DataHandle(rhs)
        {}

        /**
         * Assignment operator
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     source QtDataHandle
         * \return  *this
         */
        QtDataHandle& operator=(const QtDataHandle& rhs) {
            DataHandle::operator=(rhs);
            return *this;
        }

        /**
         * Destructor, decrease the reference count of the managed AbstractData.
         */
        virtual ~QtDataHandle() {
        }
    };
}

Q_DECLARE_METATYPE(campvis::QtDataHandle);

#endif // QTQtDataHandle_H__
