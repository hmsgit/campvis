%module campvis
%include std_string.i
%include tgt.i
%{
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/visualizationprocessor.h"
%}


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

        const std::string getName() const = 0;
    };

    class VisualizationProcessor : public AbstractProcessor {
    public:
        explicit VisualizationProcessor(IVec2Property* viewportSizeProp);
        ~VisualizationProcessor();
    };

    %nodefaultctor AutoEvaluationPipeline;

    class AutoEvaluationPipeline {
    public:
        virtual void addProcessor(AbstractProcessor* processor);
        virtual ~AutoEvaluationPipeline();
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

%luacode {
  function campvis.newPipeline (o)
    o = o or {}   -- create object if user does not provide one
    setmetatable(o, {__index = instance})
    return o
  end

  print("Module campvis loaded")
}
