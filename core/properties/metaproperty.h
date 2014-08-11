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

#ifndef METAPROPERTY_H__
#define METAPROPERTY_H__


#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

namespace campvis {
    /**
     * Property wrapping around a bunch of other properties.
     * Useful either for grouping properties or for wrapping around entire property collections.
     * 
     * \note    ATTENTION: Even though MetaProperty derives from HasPropertyCollection, it does 
     *                     neither take ownership of its wrapped properties, nor does is (de)initialize
     *                     or (un)lock them. This has to be done by the owners of the wrapped 
     *                     properties.
     */
    class CAMPVIS_CORE_API MetaProperty : public AbstractProperty, public HasPropertyCollection {
    public:
        /**
         * Creates a new MetaProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         */
        MetaProperty(const std::string& name, const std::string& title);

        /**
         * Virtual Destructor
         **/
        virtual ~MetaProperty();

        /// \see AbstractProperty::deinit
        virtual void deinit();

        /// \see HasPropertyCollection::onPropertyChanged
        virtual void onPropertyChanged(const AbstractProperty* prop);

        /**
         * Adds all properties in \a pc to this meta property.
         * \param   pc  PropertyCollection to add
         */
        void addPropertyCollection(HasPropertyCollection& pc);

    protected:

        static const std::string loggerCat_;
    };

}

#endif // METAPROPERTY_H__
