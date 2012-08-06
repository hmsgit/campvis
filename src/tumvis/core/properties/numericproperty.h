#ifndef NUMERICPROPERTY_H__
#define NUMERICPROPERTY_H__

#include "core/properties/genericproperty.h"

namespace TUMVis {
    /**
     * Interface for numeric properties.
     * Defines methods for incrementing and decrementing the property's value.
     */
    class INumericProperty {
    public:
        /// Increments the value of the property.
        virtual void increment() = 0;
        /// Decrements the value of the property.
        virtual void decrement() = 0;
    };

    /**
     * Generic class for numeric properties.
     * Numeric properties manage a minimum and maximum value and ensure, that the property's 
     * value is within these bounds.
     *
     * \tparam  T   Base type of the property's value.
     */
    template<typename T>
    class NumericProperty : public GenericProperty<T>, public INumericProperty {
    public:
        /**
         * Creates a new NumericProperty.
         * \param name      Property name
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value of the property
         * \param minValue  Minimum value for this property
         * \param maxValue  Maximum value for this property
         * \param il        Invalidation level that this property triggers
         */
        NumericProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            const T& minValue,
            const T& maxValue,
            InvalidationLevel il = InvalidationLevel::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~NumericProperty();


        /**
         * Adds the given property \a prop to the set of shared properties.
         * All shared properties will be changed when this property changes.
         * \note        Make sure not to build circular sharing or you will encounter endless loops!
         * \param prop  Property to add, must be of the same type as this property.
         */
        virtual void addSharedProperty(AbstractProperty* prop);

        /**
         * Returns the minimum value of this property.
         * \return _minValue
         */
        const T& getMinValue() const;

        /**
         * Returns the minimum value of this property.
         * \return _minValue
         */
        const T& getMaxValue() const;

        /**
         * Sets the minimum value if this property.
         * \param   value   New minimum value for this property.
         */
        virtual void setMinValue(const T& value);

        /**
         * Sets the minimum value if this property.
         * \param   value   New minimum value for this property.
         */
        virtual void setMaxValue(const T& value);

        /**
         * Increments the value of this property.
         */
        virtual void increment();

        /**
         * Decrements the value of this property.
         */
        virtual void decrement();

        /// Signal emitted, when the property's minimum or maximum value changes.
        sigslot::signal1<const AbstractProperty*> s_minMaxChanged;

    protected:

        /**
         * Checks, whether \a value is a valid value for this property.
         * Default implementation always returns true, subclasses can overwrite this method to fit their needs.
         * \param   value   value to validate.
         * \return  true if \a value is a valid value for this property.
         */
        virtual T validateValue(const T& value);

        
        T _value;                               ///< value of the property
        T _backBuffer;                          ///< back buffer for values when property is in use
        T _minValue;                            ///< Minimum value for this property
        T _maxValue;                            ///< Maximum value for this property

        static const std::string loggerCat_;
    };

// = Typedefs =====================================================================================

    typedef NumericProperty<int> IntProperty;
    typedef NumericProperty<float> FloatProperty;

// = Template Implementation ======================================================================

    template<typename T>
    TUMVis::NumericProperty<T>::NumericProperty(const std::string& name, const std::string& title, const T& value, const T& minValue, const T& maxValue, InvalidationLevel il /*= InvalidationLevel::INVALID_RESULT*/)
        : GenericProperty<T>(name, title, value, il)
        , _minValue(minValue)
        , _maxValue(maxValue)
    {

    }

    template<typename T>
    TUMVis::NumericProperty<T>::~NumericProperty() {
    }

    template<typename T>
    void TUMVis::NumericProperty<T>::addSharedProperty(AbstractProperty* prop) {
        // make type check first, then call base method.
        tgtAssert(prop != 0, "Shared property must not be 0!");
        if (NumericProperty<T>* tmp = dynamic_cast< NumericProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(getValue());
            return;
        }
        tgtAssert(false, "Shared property must be of the same type as this property!");
    }

    template<typename T>
    T TUMVis::NumericProperty<T>::validateValue(const T& value) {
        if (value >= _minValue && value <= _maxValue)
            return value;
        else {
            return (value < _minValue) ? _minValue : _maxValue;
        }
    }

    template<typename T>
    const T& TUMVis::NumericProperty<T>::getMinValue() const {
        return _minValue;
    }

    template<typename T>
    const T& TUMVis::NumericProperty<T>::getMaxValue() const {
        return _maxValue;
    }

    template<typename T>
    void TUMVis::NumericProperty<T>::setMinValue(const T& value) {
        _minValue = value;
        setValue(validateValue(getValue()));
        s_minMaxChanged(this);
    }

    template<typename T>
    void TUMVis::NumericProperty<T>::setMaxValue(const T& value) {
        _maxValue = value;
        setValue(validateValue(getValue()));
        s_minMaxChanged(this);
    }

    template<typename T>
    void TUMVis::NumericProperty<T>::increment() {
        setValue(getValue() + T(1));
    }

    template<typename T>
    void TUMVis::NumericProperty<T>::decrement() {
        setValue(getValue() - T(1));
    }
}

#endif // NUMERICPROPERTY_H__
