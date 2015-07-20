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

#ifndef OPTIONPROPERTY_H__
#define OPTIONPROPERTY_H__

#include "cgt/vector.h"
#include "core/properties/numericproperty.h"

#include <string>
#include <vector>

namespace campvis {

    /**
     * Abstract Base class for Option properties, so that they can easily be identified in widget factory.
     */
    class AbstractOptionProperty : public IntProperty {
    public:
        /**
         * Creates a new AbstractOptionProperty.
         * \param name      Property name
         * \param title     Property title (e.g. used for GUI)
         */
        AbstractOptionProperty(const std::string& name, const std::string& title)
            : IntProperty(name, title, -1, -1, -1, 1)
        {            
        };

        /**
         * Pure virtual destructor.
         */
        virtual ~AbstractOptionProperty() {};
        
        /**
         * Returns the id of the currently selected option.
         * \return  _options[_value]._id
         */
        virtual const std::string& getOptionId() = 0;

        /**
         * Sets the selected option to the first option with the given id.
         * If no such option is found, the selected option will not change.
         * \param   id  Id of the option to select.
         */
        virtual void selectById(const std::string& id) = 0;

        /**
         * Returns all Options als pair of std::strings.
         * \return  A vector of the options encoded as pair (id, title).
         */
        virtual std::vector< std::pair<std::string, std::string> > getOptionsAsPairOfStrings() const = 0;
    };

    template<typename T>
    struct GenericOption {
    public:
        GenericOption(const std::string& id, const std::string& title, T value)
            : _id(id)
            , _title(title)
            , _value(value)
        {};
        
        std::string _id;
        std::string _title;
        T _value;
    };

    template<>
    struct GenericOption<std::string> {
    public:
        GenericOption(const std::string& id, const std::string& title, const std::string& value)
            : _id(id)
            , _title(title)
            , _value(value)
        {};

        GenericOption(const std::string& id, const std::string& title)
            : _id(id)
            , _title(title)
            , _value(id)
        {};

        std::string _id;
        std::string _title;
        std::string _value;
    };

    /**
     * Property for selecting a single item of a set of generic options.
     * \tparam  T   Type of the options.
     */
    template<typename T>
    class GenericOptionProperty : public AbstractOptionProperty {
    public:
        /**
         * Creates a new GenericOptionProperty.
         * \param name      Property name
         * \param title     Property title (e.g. used for GUI)
         * \param options   Array of the options for this property, must not be 0, must not be empty.
         * \param count     Number of items in \a options (number of options), must be greater 0.

         */
        GenericOptionProperty(
            const std::string& name, 
            const std::string& title, 
            const GenericOption<T>* options,
            int count);

        /**
         * Destructor
         */
        virtual ~GenericOptionProperty();


        /**
         * Returns the vector of options of this property.
         * \return  _options
         */
        const std::vector< GenericOption<T> >& getOptions() const;

        /// \see AbstractOptionProperty::getOptionsAsPairOfStrings()
        virtual std::vector< std::pair<std::string, std::string> > getOptionsAsPairOfStrings() const;

        /**
         * Returns the currently selected option.
         * \return  _options[_value]
         */
        const GenericOption<T>& getOption() const;
        
        /**
         * Returns the value of the currently selected option.
         * \return  _options[_value]._value
         */
        T getOptionValue() const;

        /**
         * Returns the id of the currently selected option.
         * \return  _options[_value]._id
         */
        const std::string& getOptionId();


        /**
         * Sets the selected option to the first option with the given id.
         * If no such option is found, the selected option will not change.
         * \param   id  Id of the option to select.
         */
        void selectById(const std::string& id);

        /**
         * Sets the selected option to the option with the given index.
         * \param   index   Index of the option to select in the option array.
         */
        void selectByIndex(int index);

        /**
         * Sets the selected option to \a option.
         * \param   option  Option to set.
         */
        void selectByOption(T option);
        
        /** 
         * Add a new Option
         * \param option New option to add
         * returns Number of items
         */
        size_t addOption(GenericOption<T> option);

        /** 
         * Update current option
         * \param copy Data to copy from
         * returns Number of items
         */
        size_t updateCurrent(GenericOption<T> copy);
        size_t updateCurrent(T& copy);
        
        /** 
         * Remove current copy, except the last one
         * returns Number of items
         */
        size_t removeCurrent();

        /** 
         * Returns a non-const reference of current option's value
         * \param index Position to 
         * \note With great power comes great responsibility
         * returns Number of items
         */
        T& getOptionReference(int index = -1);

        /** 
         * Get Option count
         * returns Number of items
         */
        size_t getOptionCount();

    protected:
        std::vector< GenericOption<T> > _options;
    };

// = Template Implementation ======================================================================

    template<typename T>
    campvis::GenericOptionProperty<T>::GenericOptionProperty(const std::string& name, const std::string& title, const GenericOption<T>* options, int count)
        : AbstractOptionProperty(name, title)
    {
        _options.assign(options, options + count);
        setMaxValue(std::max(count - 1, 0));
        setValue(0);
        setMinValue(0);
    }

    template<typename T>
    campvis::GenericOptionProperty<T>::~GenericOptionProperty() {

    }

    template<typename T>
    const std::vector< GenericOption<T> >& campvis::GenericOptionProperty<T>::getOptions() const {
        return _options;
    }

    template<typename T>
    std::vector< std::pair<std::string, std::string> > campvis::GenericOptionProperty<T>::getOptionsAsPairOfStrings() const {
        std::vector< std::pair<std::string, std::string> > toReturn;
        for (typename std::vector< GenericOption<T> >::const_iterator it = _options.begin(); it != _options.end(); ++it) {
            toReturn.push_back(std::make_pair(it->_id, it->_title));
        }
        return toReturn;
    }

    template<typename T>
    const GenericOption<T>& campvis::GenericOptionProperty<T>::getOption() const {
        return _options[_value];
    }

    template<typename T>
    T campvis::GenericOptionProperty<T>::getOptionValue() const {
        return _options[_value]._value;
    }

    template<typename T>
    const std::string& campvis::GenericOptionProperty<T>::getOptionId() {
        return _options[_value]._id;
    }

    template<typename T>
    void campvis::GenericOptionProperty<T>::selectById(const std::string& id) {
        for (size_t i = 0; i < _options.size(); ++i) {
            if (_options[i]._id == id) {
                setValue(static_cast<int>(i));
                return;
            }
        }
        LERROR("Could not find option with id '" + id + "'");
    }

    template<typename T>
    void campvis::GenericOptionProperty<T>::selectByIndex(int index) {
        cgtAssert(index >= 0 && index < static_cast<int>(_options.size()), "Index out of bounds.");
        setValue(index);
    }

    template<typename T>
    void campvis::GenericOptionProperty<T>::selectByOption(T option) {
        for (size_t i = 0; i < _options.size(); ++i) {
            if (_options[i]._value == option) {
                setValue(static_cast<int>(i));
                return;
            }
        }
        LERROR("Could not find specified option.");
    }

    template<typename T>
    size_t campvis::GenericOptionProperty<T>::addOption(GenericOption<T> option) {
        this->_options.push_back(option);
        setMaxValue(static_cast<int>(this->_options.size()) - 1);
        return this->_options.size();
    }

    template<typename T>
    size_t campvis::GenericOptionProperty<T>::removeCurrent() {
        size_t index = getValue();
        if (index == 0) return 0;
        if (index <= this->_options.size()-1)
            this->_options.erase(this->_options.begin() + index);
        selectByIndex((index <= this->_options.size() - 1) ? static_cast<int>(index) : static_cast<int>(this->_options.size() - 1));
        return (index <= this->_options.size() - 1) ? index : this->_options.size() - 1;
    }

    template<typename T>
    size_t campvis::GenericOptionProperty<T>::updateCurrent(GenericOption<T> copy) {
        GenericOption<T>* it = &this->_options[getValue()];
        it->_id = copy._id;
        it->_title = copy._title;
        it->_value = copy._value;
        return this->_options.size();
    }

    template<typename T>
    size_t campvis::GenericOptionProperty<T>::updateCurrent(T& copy) {
        GenericOption<T>* it = &this->_options[getValue()];
        it->_value = copy;
        return this->_options.size();
    }

    template<typename T>
    T& campvis::GenericOptionProperty<T>::getOptionReference(int index) {
        return (index == -1) ? _options[_value]._value : _options[index]._value;
    }

    template<typename T>
    size_t campvis::GenericOptionProperty<T>::getOptionCount() {
        return _options.size();
    }
}

#endif // OPTIONPROPERTY_H__
