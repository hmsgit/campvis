#ifndef ABSTRACTPROPERTY_H__
#define ABSTRACTPROPERTY_H__

#include "tgt/logmanager.h"
#include "core/tools/invalidationlevel.h"
#include "core/tools/observer.h"

#include <set>
#include <string>
#include <vector>

namespace TUMVis {
    // TODO:    Hopefully this forward declaration works for the template definition.
    //          But as its only used as pointer within the template, it should be okay.
    class AbstractProperty;

    /**
     * Observer Arguments for Property observers.
     */
    struct PropertyObserverArgs : public GenericObserverArgs<AbstractProperty> {
        /**
         * Creates new PropertyObserverArgs.
         * \param subject               Subject that emits the notification
         * \param invalidationLevel     Invalidation level of that property
         */
        PropertyObserverArgs(const AbstractProperty* subject, InvalidationLevel invalidationLevel)
            : GenericObserverArgs<AbstractProperty>(subject)
            , _invalidationLevel(invalidationLevel)
        {}

        InvalidationLevel _invalidationLevel;       ///< Invalidation level of that property
    };


    /**
     * Abstract base class for TUMVis Property.
     * 
     * \todo    Add PropertyWidgets, add clone()?
     *          Think about a reasonable locking mechanism and implement that
     */
    class AbstractProperty : public GenericObservable<PropertyObserverArgs> {
    public:
        /**
         * Creates a new AbstractProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param il        Invalidation level that this property triggers
         */
        AbstractProperty(const std::string& name, const std::string& title, InvalidationLevel il = InvalidationLevel::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractProperty();


        /**
         * Returns the property name.
         * \return  _name
         */
        const std::string& getName();

        /**
         * Returns the property title (e.g. used for GUI).
         * \return  _title
         */
        const std::string& getTitle();

        /**
         * Returns the invalidation level that this property triggers.
         * \return  _invalidationLevel
         */
        const InvalidationLevel& getInvalidationLevel() const;

        /**
         * Sets the invalidation level that this property triggers.
         * \param il    New invalidation level that this property triggers.
         */
        void setInvalidationLevel(const InvalidationLevel& il);


        /**
         * Adds the given property \a prop to the set of shared properties.
         * All shared properties will be changed when this property changes.
         * Overload in subclasses to make sure that shared properties are of the same type.
         * \note        Make sure not to build circular sharing or you will encounter endless loops!
         * \param prop  Property to add.
         */
        virtual void addSharedProperty(AbstractProperty* prop);

        /**
         * Removes the given property \a prop from the set of shared properties.
         * \param prop  Property to remove.
         */
        void removeSharedProperty(AbstractProperty* prop);

        /**
         * Returns the list of shared properties, which will be changed when this property changes.
         * \note    Property sharing only works in one direction, i.e. shared properties act as child properties.
         *          Make sure not to build circular sharing structures or you will encounter endless loops.
         * \return  _sharedProperties
         */
        const std::set<AbstractProperty*>& getSharedProperties() const;


    protected:
        // DO NOT REMOVE THE CONSTNESS OF _name. PropertyCollection relies on it!
        const std::string _name;                ///< Property name (unchangable on purpose!)
        std::string _title;                     ///< Property title (e.g. used for GUI)
        InvalidationLevel _invalidationLevel;   ///< Invalidation level that this property triggers

        /**
         * List of shared properties that will be changed when this property changes.
         * \note    Property sharing only works in one direction, i.e. shared properties act as child properties.
         *          Make sure not to build circular sharing structures or you will encounter endless loops.
         */
        std::set<AbstractProperty*> _sharedProperties;

        static const std::string loggerCat_;
    };

}

#endif // ABSTRACTPROPERTY_H__
