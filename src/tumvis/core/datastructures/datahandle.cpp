// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
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

#include "datahandle.h"

#include "tgt/assert.h"
#include "core/datastructures/abstractdata.h"

namespace TUMVis {
    DataHandle::DataHandle(AbstractData* data) 
        : _data(data)
        , _timestamp(clock())
    {
        init();
    }

    DataHandle::DataHandle(const DataHandle& rhs) 
        : _data(rhs._data)
    {
        init();
    }

    DataHandle& DataHandle::operator=(const DataHandle& rhs) {
        if (_data != rhs._data) {
            AbstractData* oldData = _data;
            _data = rhs._data;
            init();
            if (oldData) {
                oldData->removeReference();
            }
        }

        return *this;
    }

    DataHandle::~DataHandle() {
        if (_data)
            _data->removeReference();
    }


    const AbstractData* DataHandle::getData() const {
        return _data;
    }

    AbstractData* DataHandle::getData() {
        if (_data->isShared())
            _data = _data->clone();

        _data->markUnsharable();
        return _data;
    }

    void DataHandle::init() {
        if (_data == 0)
            return;

        if (! _data->isShareable())
            _data = _data->clone(); // TODO: update timestamp?
        _data->addReference();
    }

    clock_t DataHandle::getTimestamp() const {
        return _timestamp;
    }

}