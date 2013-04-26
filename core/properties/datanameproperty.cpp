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

#include "datanameproperty.h"

namespace campvis {

    const std::string DataNameProperty::loggerCat_ = "CAMPVis.core.datastructures.DataNameProperty";

    DataNameProperty::DataNameProperty(const std::string& name, const std::string& title, const std::string& value, DataAccessInfo access, int invalidationLevel /*= AbstractProcessor::INVALID_RESULT*/)
        : GenericProperty<std::string>(name, title, value, invalidationLevel)
        , _accessInfo(access)
    {

    }

    DataNameProperty::~DataNameProperty() {
    }

    void DataNameProperty::connect(DataNameProperty* reader) {
        tgtAssert(reader != 0, "Reader must not be 0");
        tgtAssert(_accessInfo == WRITE, "Can only connect writing properties to reading properties.");
        tgtAssert(reader->_accessInfo == READ, "Can only connect writing properties to reading properties.");

        if (_connectedReaders.insert(reader).second)
            addSharedProperty(reader);
    }

    void DataNameProperty::disconnect(DataNameProperty* reader) {
        tgtAssert(reader != 0, "Reader must not be 0");

        if (_connectedReaders.erase(reader) > 0)
            removeSharedProperty(reader);
    }

    void DataNameProperty::issueWrite() {
        tgtAssert(_accessInfo == WRITE, "Write access not specified.");

        for (std::set<DataNameProperty*>::iterator it = _connectedReaders.begin(); it != _connectedReaders.end(); ++it) {
            (*it)->notifyReaders();
        }
    }

    void DataNameProperty::notifyReaders() {
        for (std::set<AbstractProperty*>::iterator it = _sharedProperties.begin(); it != _sharedProperties.end(); ++it) {
            // static_cast ist safe here since correct type is ensured during add of shared property
            static_cast<DataNameProperty*>(*it)->notifyReaders();
        }
        s_changed(this);
    }

}
