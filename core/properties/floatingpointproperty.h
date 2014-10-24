// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#ifndef FLOATINGPOINTPROPERTY_H__
#define FLOATINGPOINTPROPERTY_H__

#include "cgt/vector.h"
#include "core/properties/numericproperty.h"

namespace {
    /**
     * Trait determining how information about the number of significant decimal places is stored.
     *
     * Every floating point property needs to store information about the number of significant
     * decimal places of each of its components. Depending on the number of components a property
     * has, either \p int or a vector of integers is used for that.
     */
    template<typename T>
    struct FloatingPointPropertyTraits {};

    template<>
    struct FloatingPointPropertyTraits<float> {
        typedef int DecimalsType;
    };

    template<>
    struct FloatingPointPropertyTraits<cgt::vec2> {
        typedef cgt::ivec2 DecimalsType;
    };

    template<>
    struct FloatingPointPropertyTraits<cgt::vec3> {
        typedef cgt::ivec3 DecimalsType;
    };

    template<>
    struct FloatingPointPropertyTraits<cgt::vec4> {
        typedef cgt::ivec4 DecimalsType;
    };

}

namespace campvis {
    /**
     * Generic class for floating-point properties.
     * FloatingPointProperty extends NumericProperty to control how many decimal places should be
     * shown when displaying the property's value.
     *
     * \tparam  T   Base type of the property's value.
     */
    template<typename T>
    class FloatingPointProperty : public NumericProperty<T> {
    public:
        typedef typename FloatingPointPropertyTraits<T>::DecimalsType DecimalsType;

        /**
         * Creates a new FloatingPointProperty.
         * \param name      Property name
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value of the property
         * \param minValue  Minimum value for this property
         * \param maxValue  Maximum value for this property
         * \param stepValue Step value for this property
         * \param decimals  Number of significant decimal places
         */
        FloatingPointProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            const T& minValue,
            const T& maxValue,
            const T& stepValue = T(0.01f),
            const DecimalsType& decimals = DecimalsType(3));

        /**
         * Adds the given property \a prop to the set of shared properties.
         * All shared properties will be changed when this property changes.
         * \note        Make sure not to build circular sharing or you will encounter endless loops!
         * \param prop  Property to add, must be of the same type as this property.
         */
        virtual void addSharedProperty(AbstractProperty* prop);

        /**
         * Returns the number of significant decimal places.
         * \return _decimals
         */
        const DecimalsType& getDecimals() const;

        /**
         * Sets the number of significant decimal places.
         * \param   value   New number of significant decimal places.
         */
        virtual void setDecimals(const DecimalsType& decimals);

        /// Signal emitted when the number of significant decimal places of the property changes.
        sigslot::signal1<const AbstractProperty*> s_decimalsChanged;

    protected:
        DecimalsType _decimals;       ///< Number of significant decimal places

        static const std::string loggerCat_;
    };

    template<typename T>
    const std::string campvis::FloatingPointProperty<T>::loggerCat_ = "CAMPVis.core.properties.FloatingPointProperty";

// = Typedefs =====================================================================================

    typedef FloatingPointProperty<float> FloatProperty;

    typedef FloatingPointProperty<cgt::vec2> Vec2Property;
    typedef FloatingPointProperty<cgt::vec3> Vec3Property;
    typedef FloatingPointProperty<cgt::vec4> Vec4Property;

// = Template Implementation ======================================================================

    template<typename T>
    campvis::FloatingPointProperty<T>::FloatingPointProperty(const std::string& name, const std::string& title,
                                                             const T& value, const T& minValue, const T& maxValue,
                                                             const T& stepValue,
                                                             const DecimalsType& decimals /*= DecimalsType(3)*/)
        : NumericProperty<T>(name, title, value, minValue, maxValue, stepValue)
        , _decimals(decimals)
    {

    }

    template<typename T>
    void campvis::FloatingPointProperty<T>::addSharedProperty(AbstractProperty* prop) {
        // make type check first, then call base method.
        cgtAssert(prop != 0, "Shared property must not be 0!");
        if (FloatingPointProperty<T>* tmp = dynamic_cast< FloatingPointProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(GenericProperty<T>::getValue());
            return;
        }
        cgtAssert(false, "Shared property must be of the same type as this property!");
    }

    template<typename T>
    const typename campvis::FloatingPointProperty<T>::DecimalsType& campvis::FloatingPointProperty<T>::getDecimals() const {
        return _decimals;
    }

    template<typename T>
    void campvis::FloatingPointProperty<T>::setDecimals(const DecimalsType& decimals) {
        this->_decimals = decimals;

        for (std::set<AbstractProperty*>::iterator it = this->_sharedProperties.begin(); it != this->_sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of type FloatingPointProperty<T> in the
            // addSharedProperty overload. Hence, static_cast ist safe.
            FloatingPointProperty<T>* child = static_cast< FloatingPointProperty<T>* >(*it);
            child->setDecimals(decimals);
        }

        this->s_decimalsChanged.emitSignal(this);
    }
}

#endif // FLOATINGPOINTPROPERTY_H__
