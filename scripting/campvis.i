%module campvis
%include "std_string.i"
%{
#include "ext/tgt/vector.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/pipeline/abstractprocessor.h"
%}

namespace tgt {
    template<class T>
    struct Vector2 {
        Vector2(T t);
        Vector2(T t1, T t2);

        static Vector2<T> zero;
    };

    %template(ivec2) Vector2<int>;
    typedef Vector2<int> ivec2;
}

namespace campvis {
    class AbstractProcessor {
    public:
        enum InvalidationLevel {
            VALID               = 0,
            INVALID_RESULT      = 1 << 0,
            INVALID_SHADER      = 1 << 1,
            INVALID_PROPERTIES  = 1 << 2,
            FIRST_FREE_TO_USE_INVALIDATION_LEVEL = 1 << 3
        };

        virtual const std::string getName() const = 0;
    };


    class StringProperty {
    public:
        StringProperty(const std::string& name, const std::string& title, const std::string& value,
                       int invalidationLevel = AbstractProcessor::INVALID_RESULT);
        ~StringProperty();
        const std::string& getValue();
        void setValue(const std::string& value);
    };

    class DataNameProperty : public StringProperty {
    public:
        enum DataAccessInfo {
            READ,
            WRITE
        };

        DataNameProperty(const std::string& name, const std::string& title, const std::string& value,
                         DataAccessInfo access, int invalidationLevel = AbstractProcessor::INVALID_RESULT);
        virtual ~DataNameProperty();
    };

    class DataContainer {
    public:
        DataContainer(const std::string& name);
        ~DataContainer();
    };

    template<typename T>
    class NumericProperty {
    public:
        NumericProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            const T& minValue,
            const T& maxValue,
            const T& stepValue = T(1),
            int invalidationLevel = AbstractProcessor::INVALID_RESULT);

        virtual ~NumericProperty();
    };

    %template(IVec2Property) NumericProperty< tgt::Vector2<int> >;
    typedef NumericProperty< tgt::Vector2<int> > IVec2Property;
}
