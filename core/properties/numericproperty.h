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

#ifndef NUMERICPROPERTY_H__
#define NUMERICPROPERTY_H__

#include "cgt/logmanager.h"
#include "cgt/vector.h"

#include "core/coreapi.h"
#include "core/properties/genericproperty.h"

#include <limits>

namespace {
    /**
     * Helper struct for template specialization regarding scalar/vector types.
     */
    template<typename T, bool IsScalar>
    struct NumericPropertyTraits {
    };

    /**
     * Specialized traits for scalar NumericProperties.
     */
    template<typename T>
    struct NumericPropertyTraits<T, true> {
        static T validateValue(const T& value, const T& minValue, const T& maxValue) {
            if (value >= minValue && value <= maxValue) {
                return value;
            }
            else {
                if (cgt::LogManager::isInited())
                    LDEBUGC("CAMPVis.core.properties.NumericProperty", "Validating value " << value << ": Out of bounds [" << minValue << ", " << maxValue << "], clamping to range!");
                return (value < minValue) ? minValue : maxValue;
            }
        }

        static bool isNan(const T& value) {
            return (value != value);
        };
    };

    /**
     * Specialized traits for vector NumericProperties.
     */
    template<typename T>
    struct NumericPropertyTraits<T, false> {
        static T validateValue(const T& value, const T& minValue, const T& maxValue) {
            T toReturn(value);

            for (size_t i = 0; i < value.size; ++i) {
                if (toReturn[i] < minValue[i]) {
                    if (cgt::LogManager::isInited())
                        LDEBUGC("CAMPVis.core.properties.NumericProperty", "Validating value " << value << ": Out of bounds [" << minValue << ", " << maxValue << "], clamping to range!");
                    toReturn[i] = minValue[i];
                }
                else if (toReturn[i] > maxValue[i]) {
                    toReturn[i] = maxValue[i];
                }
            }
            return toReturn;
        }

        static bool isNan(const T& value) {
            bool toReturn = false;
            for (size_t i = 0; i < value.size; ++i) {
                toReturn |= (value[i] != value[i]);
            }

            return toReturn;
        };
    };

}

namespace campvis {
    /**
     * Interface for numeric properties.
     * Defines methods for incrementing and decrementing the property's value.
     */
    class CAMPVIS_CORE_API INumericProperty {
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
         * \param stepValue Step value for this property
         */
        NumericProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            const T& minValue,
            const T& maxValue,
            const T& stepValue = T(1));

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
         * Returns the step value of this property.
         * \return _stepValue
         */
        const T& getStepValue() const;

        /**
         * Sets the minimum value of this property.
         * \param   value   New minimum value for this property.
         */
        virtual void setMinValue(const T& value);

        /**
         * Sets the minimum value of this property.
         * \param   value   New minimum value for this property.
         */
        virtual void setMaxValue(const T& value);

        /**
         * Sets the step value of this property.
         * \param   value   New step value for this property.
         */
        virtual void setStepValue(const T& value);

        /**
         * Increments the value of this property.
         */
        virtual void increment();

        /**
         * Decrements the value of this property.
         */
        virtual void decrement();

        /**
         * See GenericProperty::unlock()
         * This one additionally checks for NaN values, as they break the existing code.
         */
        virtual void unlock();

        /// Signal emitted, when the property's minimum or maximum value changes.
        sigslot::signal1<const AbstractProperty*> s_minMaxChanged;

        /// Signal emitted, when the property's step value changes.
        sigslot::signal1<const AbstractProperty*> s_stepChanged;

    protected:

        /**
         * Checks, whether \a value is a valid value for this property.
         * Default implementation always returns true, subclasses can overwrite this method to fit their needs.
         * \param   value   value to validate.
         * \return  true if \a value is a valid value for this property.
         */
        virtual T validateValue(const T& value);

        T _minValue;                            ///< Minimum value for this property
        T _maxValue;                            ///< Maximum value for this property
        T _stepValue;                           ///< Step value for this property

        static const std::string loggerCat_;
    };

    template<typename T>
    const std::string campvis::NumericProperty<T>::loggerCat_ = "CAMPVis.core.properties.NumericProperty";

// = Typedefs =====================================================================================

    typedef NumericProperty<int> IntProperty;

    typedef NumericProperty<cgt::ivec2> IVec2Property;
    typedef NumericProperty<cgt::ivec3> IVec3Property;
    typedef NumericProperty<cgt::ivec4> IVec4Property;

// = Template Implementation ======================================================================

    template<typename T>
    campvis::NumericProperty<T>::NumericProperty(const std::string& name, const std::string& title, const T& value,
                                                 const T& minValue, const T& maxValue, const T& stepValue /*= T(1)*/)
        : GenericProperty<T>(name, title, value)
        , _minValue(minValue)
        , _maxValue(maxValue)
        , _stepValue(stepValue)
    {

    }

    template<typename T>
    campvis::NumericProperty<T>::~NumericProperty() {
    }

    template<typename T>
    void campvis::NumericProperty<T>::addSharedProperty(AbstractProperty* prop) {
        // make type check first, then call base method.
        cgtAssert(prop != 0, "Shared property must not be 0!");
        if (NumericProperty<T>* tmp = dynamic_cast< NumericProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(GenericProperty<T>::getValue());
            return;
        }
        cgtAssert(false, "Shared property must be of the same type as this property!");
    }

    template<typename T>
    T campvis::NumericProperty<T>::validateValue(const T& value) {
        return NumericPropertyTraits<T, std::numeric_limits<T>::is_specialized>::validateValue(value, _minValue, _maxValue);
    }

    template<typename T>
    const T& campvis::NumericProperty<T>::getMinValue() const {
        return _minValue;
    }

    template<typename T>
    const T& campvis::NumericProperty<T>::getMaxValue() const {
        return _maxValue;
    }

    template<typename T>
    const T& campvis::NumericProperty<T>::getStepValue() const {
        return _stepValue;
    }

    template<typename T>
    void campvis::NumericProperty<T>::setMinValue(const T& value) {
        this->_minValue = value;
        this->setValue(validateValue(this->_value));

        for (std::set<AbstractProperty*>::iterator it = this->_sharedProperties.begin(); it != this->_sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of type NumericProperty<T> in the addSharedProperty overload.
            // Hence, static_cast ist safe.
            NumericProperty<T>* child = static_cast< NumericProperty<T>* >(*it);
            child->setMinValue(value);
        }

        this->s_minMaxChanged.emitSignal(this);
    }

    template<typename T>
    void campvis::NumericProperty<T>::setMaxValue(const T& value) {
        this->_maxValue = value;
        this->setValue(validateValue(this->_value));

        for (std::set<AbstractProperty*>::iterator it = this->_sharedProperties.begin(); it != this->_sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of type NumericProperty<T> in the addSharedProperty overload.
            // Hence, static_cast ist safe.
            NumericProperty<T>* child = static_cast< NumericProperty<T>* >(*it);
            child->setMaxValue(value);
        }

        this->s_minMaxChanged.emitSignal(this);
    }

    template<typename T>
    void campvis::NumericProperty<T>::setStepValue(const T& value) {
        this->_stepValue = value;

        for (std::set<AbstractProperty*>::iterator it = this->_sharedProperties.begin(); it != this->_sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of type NumericProperty<T> in the addSharedProperty overload.
            // Hence, static_cast ist safe.
            NumericProperty<T>* child = static_cast< NumericProperty<T>* >(*it);
            child->setStepValue(value);
        }

        this->s_stepChanged.emitSignal(this);
    }

    template<typename T>
    void campvis::NumericProperty<T>::increment() {
        this->setValue(this->_value + this->_stepValue);
    }

    template<typename T>
    void campvis::NumericProperty<T>::decrement() {
        this->setValue(this->_value - this->_stepValue);
    }

    template<typename T>
    void campvis::NumericProperty<T>::unlock() {
        if (NumericPropertyTraits<T, std::numeric_limits<T>::is_specialized>::isNan(this->_backBuffer) && NumericPropertyTraits<T, std::numeric_limits<T>::is_specialized>::isNan(this->_value))
            AbstractProperty::unlock();
        else
            GenericProperty<T>::unlock();
    }
}

#endif // NUMERICPROPERTY_H__
