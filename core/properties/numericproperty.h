// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef NUMERICPROPERTY_H__
#define NUMERICPROPERTY_H__

#include "tgt/vector.h"
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
            if (value >= minValue && value <= maxValue)
                return value;
            else {
                return (value < minValue) ? minValue : maxValue;
            }
        }
    };

    /**
     * Specialized traits for vector NumericProperties.
     */
    template<typename T>
    struct NumericPropertyTraits<T, false> {
        static T validateValue(const T& value, const T& minValue, const T& maxValue) {
            T toReturn(value);

            for (size_t i = 0; i < value.size; ++i) {
                if (toReturn[i] < minValue[i])
                    toReturn[i] = minValue[i];
                else if (toReturn[i] > maxValue[i])
                    toReturn[i] = maxValue[i];
            }
            return toReturn;
        }
    };

}

namespace campvis {
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
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        NumericProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            const T& minValue,
            const T& maxValue,
            int invalidationLevel = AbstractProcessor::INVALID_RESULT);

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

        T _minValue;                            ///< Minimum value for this property
        T _maxValue;                            ///< Maximum value for this property

        static const std::string loggerCat_;
    };

    template<typename T>
    const std::string campvis::NumericProperty<T>::loggerCat_ = "CAMPVis.core.properties.NumericProperty";

// = Typedefs =====================================================================================

    typedef NumericProperty<int> IntProperty;
    typedef NumericProperty<float> FloatProperty;

    typedef NumericProperty<tgt::ivec2> IVec2Property;
    typedef NumericProperty<tgt::vec2> Vec2Property;
    typedef NumericProperty<tgt::ivec3> IVec3Property;
    typedef NumericProperty<tgt::vec3> Vec3Property;
    typedef NumericProperty<tgt::ivec4> IVec4Property;
    typedef NumericProperty<tgt::vec4> Vec4Property;

// = Template Implementation ======================================================================

    template<typename T>
    campvis::NumericProperty<T>::NumericProperty(const std::string& name, const std::string& title, const T& value, const T& minValue, const T& maxValue, int invalidationLevel /*= AbstractProcessor::INVALID_RESULT*/)
        : GenericProperty<T>(name, title, value, invalidationLevel)
        , _minValue(minValue)
        , _maxValue(maxValue)
    {

    }

    template<typename T>
    campvis::NumericProperty<T>::~NumericProperty() {
    }

    template<typename T>
    void campvis::NumericProperty<T>::addSharedProperty(AbstractProperty* prop) {
        // make type check first, then call base method.
        tgtAssert(prop != 0, "Shared property must not be 0!");
        if (NumericProperty<T>* tmp = dynamic_cast< NumericProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(GenericProperty<T>::getValue());
            return;
        }
        tgtAssert(false, "Shared property must be of the same type as this property!");
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
    void campvis::NumericProperty<T>::setMinValue(const T& value) {
        this->_minValue = value;
        this->setValue(validateValue(this->_value));

        for (std::set<AbstractProperty*>::iterator it = this->_sharedProperties.begin(); it != this->_sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of type NumericProperty<T> in the addSharedProperty overload.
            // Hence, static_cast ist safe.
            NumericProperty<T>* child = static_cast< NumericProperty<T>* >(*it);
            child->setMinValue(value);
        }

        this->s_minMaxChanged(this);
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

        this->s_minMaxChanged(this);
    }

    template<typename T>
    void campvis::NumericProperty<T>::increment() {
        this->setValue(this->_value + T(1));
    }

    template<typename T>
    void campvis::NumericProperty<T>::decrement() {
        this->setValue(this->_value - T(1));
    }
}

#endif // NUMERICPROPERTY_H__
