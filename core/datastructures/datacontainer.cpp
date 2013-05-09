// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "datacontainer.h"

#include "tgt/assert.h"
#include "core/datastructures/abstractdata.h"

namespace campvis {
    const std::string DataContainer::loggerCat_ = "CAMPVis.core.datastructures.DataContainer";

    DataContainer::DataContainer() {

    }

    DataContainer::~DataContainer() {
        _handles.clear();
    }

    DataHandle DataContainer::addData(const std::string& name, AbstractData* data) {
        tgtAssert(data != 0, "The Data must not be 0.");
        DataHandle dh(data);
        addDataHandle(name, dh);
        return dh;
    }

    void DataContainer::addDataHandle(const std::string& name, const DataHandle& dh) {
        tgtAssert(dh.getData() != 0, "The data in the DataHandle must not be 0!");
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            std::map<std::string, DataHandle>::iterator it = _handles.lower_bound(name);
            if (it != _handles.end() && it->first == name) {
                it->second = dh;
            }
            else {
                _handles.insert(it, std::make_pair(name, dh));
            }
        }
        s_dataAdded(name, dh);
        s_changed();
    }

    bool DataContainer::hasData(const std::string& name) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        return (_handles.find(name) != _handles.end());
    }

    DataHandle DataContainer::getData(const std::string& name) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        std::map<std::string, DataHandle>::const_iterator it = _handles.find(name);
        if (it == _handles.end())
            return DataHandle(0);
        else
            return it->second;
    }

    void DataContainer::removeData(const std::string& name) {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        std::map<std::string, DataHandle>::iterator it = _handles.find(name);
        if (it != _handles.end()) {
            _handles.erase(it);
        }
    }

    std::vector< std::pair< std::string, DataHandle> > DataContainer::getDataHandlesCopy() const {
        std::vector< std::pair< std::string, DataHandle> > toReturn;
        toReturn.reserve(_handles.size());

        tbb::spin_mutex::scoped_lock lock(_localMutex);
        for (std::map<std::string, DataHandle>::const_iterator it = _handles.begin(); it != _handles.end(); ++it) {
            toReturn.push_back(std::make_pair(it->first, it->second));
        }

        return toReturn;
    }

    std::map<std::string, DataHandle> DataContainer::getHandlesCopy() const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        return _handles;
    }

}