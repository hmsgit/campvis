#ifndef GENERICPROPERTY_H__
#define GENERICPROPERTY_H__

#include "tbb/include/tbb/spin_mutex.h"
#include "tgt/logmanager.h"
#include "core/properties/abstractproperty.h"

namespace TUMVis {

    /**
     * Generic class for value-based properties.
     *
     * \tparam  T   Base type of the property's value.
     * \todo    Add PropertyWidgets, review use of mutex
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
         * Adds the given property \a prop to the set of shared properties.
         * All shared properties will be changed when this property changes.
         * \note        Make sure not to build circular sharing or you will encounter endless loops!
         * \param prop  Property to add, must be of the same type as this property.
         */
        virtual void addSharedProperty(AbstractProperty* prop);

        /**
         * Returns the current value of this property.
         * \return _value
         */
        const T& getValue() const;

        /**
         * On successful validation it sets the property value to \a value and notifies all observers.
         * Depending on the current lock state of the property, the value will either be written to the front or back buffer.
         * \sa GenericProperty::validateValue
         * \param value     New value for this property.
         */
        virtual void setValue(const T& value);


        /**
         * Locks the property and marks it as "in use". All changes to its value will be written to
         * the back buffer. The buffers will be swapped on unlocking the property.
         * \sa  GenericProperty::unlock
         */
        virtual void lock();

        /**
         * Unlocks the property. If the back buffer has changed, the changes will be written to the front
         * buffer and all observers will be notified.
         * \sa  GenericProperty::lock
         */
        virtual void unlock();


    protected:

        /**
         * Adapts \a value, so that is is a valid value for this property.
         * Default implementation does nothing and always returns \a value, subclasses can overwrite this method to fit their needs.
         * \param   value   value to validate.
         */
        virtual T validateValue(const T& value);

        /**
         * Sets the property value to \a value and notifies all observers.
         * \note    _localMutex has to be acquired before calling!
         * \param   value   New value for _value.
         */
        void setFrontValue(const T& value);

        /**
         * Sets the property's back buffer value to \a value.
         * \note    _localMutex has to be acquired before calling!
         * \param   value   New value for _backBuffer.
         */
        void setBackValue(const T& value);

        
        T _value;                               ///< value of the property
        T _backBuffer;                          ///< back buffer for values when property is in use

        static const std::string loggerCat_;
    };

// = Typedefs =====================================================================================

    typedef GenericProperty<std::string> StringProperty;

// = Template Implementation ======================================================================

    template<typename T>
    TUMVis::GenericProperty<T>::GenericProperty(const std::string& name, const std::string& title, const T& value, InvalidationLevel il /*= InvalidationLevel::INVALID_RESULT*/) 
        : AbstractProperty(name, title, il)
        , _value(value)
        , _backBuffer(value)
    {
    }

    template<typename T>
    TUMVis::GenericProperty<T>::~GenericProperty() {

    }

    template<typename T>
    void TUMVis::GenericProperty<T>::addSharedProperty(AbstractProperty* prop) {
        // make type check first, then call base method.
        tgtAssert(prop != 0, "Shared property must not be 0!");
        if (GenericProperty<T>* tmp = dynamic_cast< GenericProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(getValue());
            return;
        }
        tgtAssert(false, "Shared property must be of the same type as this property!");
    }

    template<typename T>
    const T& TUMVis::GenericProperty<T>::getValue() const {
        return _value;
    }

    template<typename T>
    void TUMVis::GenericProperty<T>::setValue(const T& value) {
        T vv = validateValue(value);
        tbb::spin_mutex::scoped_lock lock(_localMutex);

        if (_inUse)
            setBackValue(vv);
        else {
            setFrontValue(vv);
            setBackValue(vv);
        }
    }

    template<typename T>
    void TUMVis::GenericProperty<T>::lock() {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _inUse = true;
    }

    template<typename T>
    void TUMVis::GenericProperty<T>::unlock() {
        tbb::spin_mutex::scoped_lock lock(_localMutex);

        if (_backBuffer != _value)
            setFrontValue(_backBuffer);
        _inUse = false;
    }

    template<typename T>
    void TUMVis::GenericProperty<T>::setFrontValue(const T& value) {
        _value = value;
        // TODO:    think about the correct/reasonable order of observer notification
        //          thread-safety might play a role thereby...
        for (std::set<AbstractProperty*>::iterator it = _sharedProperties.begin(); it != _sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of type GenericProperty<T> in the addSharedProperty overload.
            // Hence, static_cast ist safe.
            GenericProperty<T>* child = static_cast< GenericProperty<T>* >(*it);
            child->setValue(value);
        }
        s_changed(this);
    }

    template<typename T>
    void TUMVis::GenericProperty<T>::setBackValue(const T& value) {
        _backBuffer = value;
    }

    template<typename T>
    T TUMVis::GenericProperty<T>::validateValue(const T& value) {
        return value;
    }

    template<typename T>
    const std::string TUMVis::GenericProperty<T>::loggerCat_ = "TUMVis.core.datastructures.GenericProperty";
}

#endif // GENERICPROPERTY_H__
