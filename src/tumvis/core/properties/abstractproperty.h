#ifndef ABSTRACTPROPERTY_H__
#define ABSTRACTPROPERTY_H__

#include "tgt/logmanager.h"
#include "core/tools/invalidationlevel.h"
#include "core/tools/observer.h"

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


    protected:
        // DO NOT REMOVE THE CONSTNESS OF _name. PropertyCollection relies on it!
        const std::string _name;                ///< Property name (unchangable on purpose!)
        std::string _title;                     ///< Property title (e.g. used for GUI)
        InvalidationLevel _invalidationLevel;   ///< Invalidation level that this property triggers

        static const std::string loggerCat_;
    };

}

#endif // ABSTRACTPROPERTY_H__
