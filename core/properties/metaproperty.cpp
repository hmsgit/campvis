// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "metaproperty.h"

namespace campvis {

    const std::string MetaProperty::loggerCat_ = "CAMPVis.core.datastructures.MetaProperty";

    MetaProperty::MetaProperty(const std::string& name, const std::string& title)
        : AbstractProperty(name, title)
    {
    }

    MetaProperty::~MetaProperty() {
    }

    void MetaProperty::deinit() {
        for (size_t i = 0; i < _properties.size(); ++i) {
            removeProperty(*_properties[i]);
        }
    }

    void MetaProperty::onPropertyChanged(const AbstractProperty* prop) {
        s_changed.emitSignal(this);
    }

    void MetaProperty::addPropertyCollection(HasPropertyCollection& pc) {
        PropertyCollection& c = pc.getProperties();
        for (std::vector<AbstractProperty*>::const_iterator it = c.begin(); it != c.end(); ++it) {
            addProperty(**it);
        }
    }


}
