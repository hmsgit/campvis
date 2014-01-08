// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef PROPERTYCOLLECTION_H__
#define PROPERTYCOLLECTION_H__

#include "sigslot/sigslot.h"
#include <vector>
#include <string>


namespace campvis {
    class AbstractProperty;

    /// A PropertyCollection wraps around a bunch of properties - currently its just a typedef...
    typedef std::vector<AbstractProperty*> PropertyCollection;

    /**
     * Abstract base class for classes having a PropertyCollection.
     */
    class HasPropertyCollection : public sigslot::has_slots<> {
    public:
        HasPropertyCollection();

        virtual ~HasPropertyCollection() = 0;

        /**
         * Registers \a prop as property . Registered properties can be accessed from the outside,
         * e.g. via getProperty(). An already existing property with the same name will be replaced.
         * \note        Processor does _not_ take ownership of the property. Hence, make sure to
         *              unregister the property before destroying/deleting it.
         * \param prop  Property to register
         */
        void addProperty(AbstractProperty* prop);

        /**
         * Unregisters \a prop from this processor.
         * \sa      HasPropertyCollection::addProperty
         * \param   prop  Property to unregister.
         */
        void removeProperty(AbstractProperty* prop);

        /**
         * Returns the property with the given name \a name.
         * If no such property exists, the result will be 0.
         * \param name  Name of the property to return.
         * \return      The property named \a name, 0 if no such property exists.
         */
        AbstractProperty* getProperty(const std::string& name) const;

        /**
         * Returns the PropertyCollection of this processor.
         * \note    There is also a const overload of this method.
         * \return  _properties
         */
        PropertyCollection& getProperties();

        /**
         * Returns the list of all registered properties.
         * \note    Please do not mess with the non-const pointers in the vector.
         * \return  _properties
         */
        const PropertyCollection& getProperties() const;

        /**
         * Calls AbstractProperty::lock() for every registered property.
         * \sa  AbstractProperty::lock, PropertyCollection::unlockAllProperties
         */
        void lockAllProperties();

        /**
         * Calls AbstractProperty::unlock() for every registered property.
         * \sa  AbstractProperty::unlock, PropertyCollection::lockAllProperties
         */
        virtual void unlockAllProperties();

        /**
         * Initializes all properties.
         */
        virtual void initAllProperties();
        
        /**
         * Deinitializes all properties.
         */
        virtual void deinitAllProperties();

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

    protected:
        /**
         * Searches _properties for a property named \a name.
         * \param name  Property name to search for.
         * \return      An iterator to the search result, _properties.end() if no such property was found.
         */
        PropertyCollection::iterator findProperty(const std::string& name);

        /**
         * Searches _properties for a property named \a name.
         * \param name  Property name to search for.
         * \return      An iterator to the search result, _properties.end() if no such property was found.
         */
        PropertyCollection::const_iterator findProperty(const std::string& name) const;

        PropertyCollection _properties;     ///< list of all registered properties
    };

}

#endif // PROPERTYCOLLECTION_H__
