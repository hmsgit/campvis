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

#ifndef OPTIONPROPERTY_H__
#define OPTIONPROPERTY_H__

#include "tgt/vector.h"
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
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        AbstractOptionProperty(const std::string& name, const std::string& title, int invalidationLevel = AbstractProcessor::INVALID_RESULT)
            : IntProperty(name, title, -1, -1, -1, 1, invalidationLevel)
        {            
        };

        /**
         * Pure virtual destructor.
         */
        virtual ~AbstractOptionProperty() {};

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
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        GenericOptionProperty(
            const std::string& name, 
            const std::string& title, 
            const GenericOption<T>* options,
            int count,
            int invalidationLevel = AbstractProcessor::INVALID_RESULT);

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

    protected:
        std::vector< GenericOption<T> > _options;
    };

// = Template Implementation ======================================================================

    template<typename T>
    campvis::GenericOptionProperty<T>::GenericOptionProperty(const std::string& name, const std::string& title, const GenericOption<T>* options, int count, int invalidationLevel /*= AbstractProcessor::INVALID_RESULT*/)
        : AbstractOptionProperty(name, title, invalidationLevel)
    {
        tgtAssert(options != 0, "Pointer to options array must not be 0.")
        tgtAssert(count > 0, "The number of options must be greater 0.");

        _options.assign(options, options + count);
        setMinValue(0);
        setMaxValue(count - 1);
        setValue(0);
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
        tgtAssert(index > 0 && index < _options.size(), "Index out of bounds.");
        setValue(index);
    }

}

#endif // OPTIONPROPERTY_H__
