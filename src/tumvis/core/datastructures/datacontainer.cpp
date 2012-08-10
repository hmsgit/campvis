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

#include "datacontainer.h"

#include "tgt/assert.h"
#include "core/datastructures/abstractdata.h"

namespace TUMVis {
    const std::string DataContainer::loggerCat_ = "TUMVis.core.datastructures.DataContainer";

    DataContainer::DataContainer() {

    }

    DataContainer::~DataContainer() {
        // remove ownership op all owned DataHandles
        for (std::map<std::string, const DataHandle*>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            delete it->second;
        }
        _handles.clear();
    }

    const DataHandle* DataContainer::addData(const std::string& name, AbstractData* data) {
        DataHandle* dh = new DataHandle(data);
        addDataHandle(name, dh);
        return dh;
    }

    void DataContainer::addDataHandle(const std::string& name, const DataHandle* dh) {
        tgtAssert(dh != 0, "DataHandle must not be 0.");
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            std::map<std::string, const DataHandle*>::iterator it = _handles.lower_bound(name);
            if (it != _handles.end() && it->first == name) {
                delete it->second;
                it->second = dh;
            }
            else {
                _handles.insert(it, std::make_pair(name, dh));
            }
        }
        s_dataAdded(name, dh);
    }

    bool DataContainer::hasData(const std::string& name) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        return (_handles.find(name) != _handles.end());
    }

    const DataHandle* DataContainer::getData(const std::string& name) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        std::map<std::string, const DataHandle*>::const_iterator it = _handles.find(name);
        if (it == _handles.end())
            return 0;
        else
            return new DataHandle(*it->second);
    }

    void DataContainer::removeData(const std::string& name) {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        std::map<std::string, const DataHandle*>::const_iterator it = _handles.find(name);
        if (it != _handles.end()) {
            delete it->second;
            _handles.erase(it);
        }
    }
}