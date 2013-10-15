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

#ifndef GENERICPROPERTY_H__
#define GENERICPROPERTY_H__

#include <tbb/spin_mutex.h>
#include "tgt/logmanager.h"
#include "core/properties/abstractproperty.h"

namespace campvis {

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
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        GenericProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            int invalidationLevel = AbstractProcessor::INVALID_RESULT);

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
    typedef GenericProperty<std::string> StringProperty;

// = Template Implementation ======================================================================

    template<typename T>
    campvis::GenericProperty<T>::GenericProperty(const std::string& name, const std::string& title, const T& value, int invalidationLevel /*= AbstractProcessor::INVALID_RESULT*/) 
        : AbstractProperty(name, title, invalidationLevel)
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
        tgtAssert(prop != 0, "Shared property must not be 0!");
        if (GenericProperty<T>* tmp = dynamic_cast< GenericProperty<T>* >(prop)) {
            AbstractProperty::addSharedProperty(prop);
            tmp->setValue(getValue());
            return;
        }
        tgtAssert(false, "Shared property must be of the same type as this property!");
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
