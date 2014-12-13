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

#ifndef ABSTRACTPROPERTYLUA_H__
#define ABSTRACTPROPERTYLUA_H__

#include "sigslot/sigslot.h"
#include "tbb/atomic.h"
#include <QBoxLayout>
#include <QLabel>

namespace campvis {
    class AbstractProperty;
    class DataContainer;

    /**
     * Abstract base class for property luas.
     */
    class AbstractPropertyLua {
    public:
        /**
         * Creates a new Lua Property for the property \a property.
         *
         * If displayBoxed is true, the lua is displayed vertically in a QGroupBox.
         *
         * \param   property        The property the lua shall handle.
         */
        AbstractPropertyLua(AbstractProperty* property);

        /**
         * Destructor
         */
        virtual ~AbstractPropertyLua();
    public:
        virtual std::string getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) = 0;

    protected:
        AbstractProperty* _property;    ///< The property this lua handles
    };
}

#endif // ABSTRACTPROPERTYLUA_H__
