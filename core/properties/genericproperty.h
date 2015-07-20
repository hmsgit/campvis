// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef GENERICPROPERTY_H__
#define GENERICPROPERTY_H__

#include <tbb/spin_mutex.h>
#include "cgt/logmanager.h"
#include "core/properties/abstractproperty.h"

namespace campvis {

    /**
     * Generic class for value-based properties.
     *
     * \tparam  T   Base type of the property's value.
     */
    template<typename T>
    class GenericProperty : public AbstractProperty {
    public:
        /**
         * Creates a new GenericProperty.
         * \param name      Property name
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value of the property
         */
        GenericProperty(
            const std::string& name,
            const std::string& title,
            const T& value);

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

    typedef GenericProperty<bool> BoolProperty;

// = Template Implementation ======================================================================

    template<typename T>
    campvis::GenericProperty<T>::GenericProperty(const std::string& name, const std::string& title, const T& value) 
        : AbstractProperty(name, title)
        , _value(value)
        , _backBuffer(value)
    {
    }

    template<typename T>
    campvis::GenericProperty<T>::~GenericProperty() {

    }

    template<typename T>
    void campvis::GenericProperty<T>::addSharedProperty(AbstractProperty* prop) {
        // make type check first, then call base method.
        cgtAssert(prop != 0, "Shared property must not be 0!");
        if (GenericProperty<T>* tmp = dynamic_cast< GenericProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(getValue());
            return;
        }
        cgtAssert(false, "Shared property must be of the same type as this property!");
    }

    template<typename T>
    const T& campvis::GenericProperty<T>::getValue() const {
        return _value;
    }

    template<typename T>
    void campvis::GenericProperty<T>::setValue(const T& value) {
        T vv = validateValue(value);
        tbb::spin_mutex::scoped_lock lock(_localMutex);

        if (_inUse != 0)
            setBackValue(vv);
        else {
            setFrontValue(vv);
            setBackValue(vv);
        }
    }

    template<typename T>
    void campvis::GenericProperty<T>::unlock() {
        tbb::spin_mutex::scoped_lock lock(_localMutex);

        if (_backBuffer != _value)
            setFrontValue(_backBuffer);
        AbstractProperty::unlock();
    }

    template<typename T>
    void campvis::GenericProperty<T>::setFrontValue(const T& value) {
        bool valueChanged = !(_value == value);
        _value = value;
        for (std::set<AbstractProperty*>::iterator it = _sharedProperties.begin(); it != _sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of type GenericProperty<T> in the addSharedProperty overload.
            // Hence, static_cast is safe.
            GenericProperty<T>* child = static_cast< GenericProperty<T>* >(*it);
            child->setValue(value);
        }
        if (valueChanged) //dont't signal a change if it has not really changed
            s_changed.emitSignal(this);
    }

    template<typename T>
    void campvis::GenericProperty<T>::setBackValue(const T& value) {
        _backBuffer = value;
    }

    template<typename T>
    T campvis::GenericProperty<T>::validateValue(const T& value) {
        return value;
    }

    template<typename T>
    const std::string campvis::GenericProperty<T>::loggerCat_ = "CAMPVis.core.datastructures.GenericProperty";
}

#endif // GENERICPROPERTY_H__
