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

#ifndef BUTTONPROPERTY_H__
#define BUTTONPROPERTY_H__

#include "core/properties/abstractproperty.h"

namespace campvis {
    /**
     * Property that simply offers a single button.
     */
    class CAMPVIS_CORE_API ButtonProperty : public AbstractProperty {
    public:
        /**
         * Creates a new ButtonProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         */
        ButtonProperty(const std::string& name, const std::string& title);

        /**
         * Virtual Destructor
         **/
        virtual ~ButtonProperty();

        /**
         * Simulates a click event.
         */
        void click();

        /**
         * Signal emitted when button was clicked.
         */
        sigslot::signal0<> s_clicked;

    protected:

        static const std::string loggerCat_;
    };

}

#endif // BUTTONPROPERTY_H__
