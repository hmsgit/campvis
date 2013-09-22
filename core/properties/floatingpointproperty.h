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

#ifndef FLOATINGPOINTPROPERTY_H__
#define FLOATINGPOINTPROPERTY_H__

#include "tgt/vector.h"
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
    struct FloatingPointPropertyTraits<tgt::vec2> {
        typedef tgt::ivec2 DecimalsType;
    };

    template<>
    struct FloatingPointPropertyTraits<tgt::vec3> {
        typedef tgt::ivec3 DecimalsType;
    };

    template<>
    struct FloatingPointPropertyTraits<tgt::vec4> {
        typedef tgt::ivec4 DecimalsType;
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
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        FloatingPointProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            const T& minValue,
            const T& maxValue,
            const T& stepValue,
            const DecimalsType& decimals = DecimalsType(3),
            int invalidationLevel = AbstractProcessor::INVALID_RESULT);

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

    typedef FloatingPointProperty<tgt::vec2> Vec2Property;
    typedef FloatingPointProperty<tgt::vec3> Vec3Property;
    typedef FloatingPointProperty<tgt::vec4> Vec4Property;

// = Template Implementation ======================================================================

    template<typename T>
    campvis::FloatingPointProperty<T>::FloatingPointProperty(const std::string& name, const std::string& title,
                                                             const T& value, const T& minValue, const T& maxValue,
                                                             const T& stepValue,
                                                             const DecimalsType& decimals /*= DecimalsType(3)*/,
                                                             int invalidationLevel /*= AbstractProcessor::INVALID_RESULT*/)
        : NumericProperty<T>(name, title, value, minValue, maxValue, stepValue, invalidationLevel)
        , _decimals(decimals)
    {

    }

    template<typename T>
    void campvis::FloatingPointProperty<T>::addSharedProperty(AbstractProperty* prop) {
        // make type check first, then call base method.
        tgtAssert(prop != 0, "Shared property must not be 0!");
        if (FloatingPointProperty<T>* tmp = dynamic_cast< FloatingPointProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(GenericProperty<T>::getValue());
            return;
        }
        tgtAssert(false, "Shared property must be of the same type as this property!");
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

        this->s_decimalsChanged(this);
    }
}

#endif // FLOATINGPOINTPROPERTY_H__
