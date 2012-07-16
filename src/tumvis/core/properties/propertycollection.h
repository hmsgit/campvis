#ifndef PROPERTYCOLLECTION_H__
#define PROPERTYCOLLECTION_H__

#include "core/properties/abstractproperty.h"

#include <vector>
#include <string>


namespace TUMVis {

    /**
     * Wrapper for a vector of Properties.
     * Properties can be registered and then accessed via their names.
     */
    class PropertyCollection {
    public:
        PropertyCollection();

        /**
         * Registers \a prop as property of this processor. Registered properties can be accessed from
         * the outside, e.g. via getProperty(). An already existing property with the same name will 
         * be replaced.
         * \note        Processor does _not_ take ownership of the property. Hence, make sure to
         *              unregister the property before destroying/deleting it.
         * \param prop  Property to register
         */
        void addProperty(AbstractProperty* prop);

        /**
         * Unregisters \a prop from this processor.
         * \sa AbstractProcessor::addProperty
         * \param prop  Property to unregister.
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
         * Returns the list of all registered properties.
         * \note    Please do not mess with the non-const pointers in the vector.
         * \return  _properties
         */
        const std::vector<AbstractProperty*>& getProperties() const;

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

    private:
        /**
         * Searches _properties for a property named \a name.
         * \param name  Property name to search for.
         * \return      An iterator to the search result, _properties.end() if no such property was found.
         */
        std::vector<AbstractProperty*>::iterator findProperty(const std::string& name);

        /**
         * Searches _properties for a property named \a name.
         * \param name  Property name to search for.
         * \return      An iterator to the search result, _properties.end() if no such property was found.
         */
        std::vector<AbstractProperty*>::const_iterator findProperty(const std::string& name) const;

        std::vector<AbstractProperty*> _properties;     ///< list of all registered properties
    };

}

#endif // PROPERTYCOLLECTION_H__
