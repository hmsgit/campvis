#ifndef GENERICPROPERTY_H__
#define GENERICPROPERTY_H__

#include "tgt/logmanager.h"
#include "core/tools/observer.h"

#include <string>
#include <vector>

namespace TUMVis {

    /**
     * Abstract base class for TUMVis Property.
     * 
     * \todo    Add PropertyWidgets
     */
    template<typename T>
    class GenericProperty : public Observable {
    public:
        /**
         * Creates an GenericProperty.
         */
        GenericProperty();

        /**
         * Virtual Destructor
         **/
        virtual ~GenericProperty();


        const std::string& getName() {
            return _name;
        }

        const std::string& getTitle() {
            return _title;
        }

        const T getValue() const {
            return _value;
        }

        void setValue(const T& value) {
            _value = value;
            notifyObservers();
        }



    protected:

        std::string _name;
        std::string _title;

        T _value;

        static const std::string loggerCat_;
    };

}

#endif // GENERICPROPERTY_H__
