#ifndef GENERICPROPERTY_H__
#define GENERICPROPERTY_H__

#include "tgt/logmanager.h"
#include "core/properties/abstractproperty.h"

namespace TUMVis {

    /**
     * Generic class for value-based properties.
     *
     * \tparam  T   Base type of the property's value.
     * \todo    Add PropertyWidgets
     */
    template<typename T>
    class GenericProperty : public AbstractProperty {
    public:
        /**
         * Creates a new GenericProperty.
         * \param name      Property name
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value of the property
         * \param il        Invalidation level that this property triggers
         */
        GenericProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            InvalidationLevel il = InvalidationLevel::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~GenericProperty();


        /**
         * Returns the current value of this property.
         * \return _value
         */
        const T getValue() const;

        /**
         * Sets the property value to \a value and notifies all observers.
         * \param value     New value for this property.
         */
        void setValue(const T& value);


    protected:
        T _value;           ///< value of the property
        static const std::string loggerCat_;
    };

// - template implementation ----------------------------------------------------------------------

    template<typename T>
    TUMVis::GenericProperty<T>::GenericProperty(const std::string& name, const std::string& title, const T& value, InvalidationLevel il /*= InvalidationLevel::INVALID_RESULT*/) 
        : AbstractProperty(name, title, il)
        , _value(value)
    {
    }

    template<typename T>
    TUMVis::GenericProperty<T>::~GenericProperty() {

    }

    template<typename T>
    const T TUMVis::GenericProperty<T>::getValue() const {
        return _value;
    }

    template<typename T>
    void TUMVis::GenericProperty<T>::setValue(const T& value) {
        _value = value;
        notifyObservers(PropertyObserverArgs(this, _invalidationLevel));
    }

    template<typename T>
    const std::string TUMVis::GenericProperty<T>::loggerCat_ = "TUMVis.core.datastructures.GenericProperty";
}

#endif // GENERICPROPERTY_H__
