%module campvis
%include factory.i
%include std_string.i
%import "ext/tgt/bindings/tgt.i"
%{
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/geometry1dtransferfunction.h"
%}


namespace campvis {

    class AbstractProperty {
    public:
        AbstractProperty(const std::string& name, const std::string& title,
                         int invalidationLevel = AbstractProcessor::INVALID_RESULT);
        virtual ~AbstractProperty();

        virtual void init();
        virtual void deinit();

        const std::string& getName() const;
        const std::string& getTitle() const;

        int getInvalidationLevel() const;
        void setInvalidationLevel(int il);

        bool isVisible() const;
        void setVisible(bool isVisible);

        virtual void addSharedProperty(AbstractProperty* prop);
    };

    template<typename T>
    class GenericProperty : public AbstractProperty {
    public:
        GenericProperty(const std::string& name, const std::string& title, const T& value,
                        int invalidationLevel = AbstractProcessor::INVALID_RESULT);
        virtual ~GenericProperty();

        const T& getValue() const;
        virtual void setValue(const T& value);
    };

    %template(StringProperty) GenericProperty<std::string>;
    typedef GenericProperty<std::string> StringProperty;

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

    /* AbstractProcessor */

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

    /* AbstractPipeline */

    class AbstractPipeline {
    public:
        AbstractPipeline(DataContainer* dc);
        virtual ~AbstractPipeline();

        virtual const std::string getName() const = 0;
    };

    /* AutoEvaluationPipeline */

    class AutoEvaluationPipeline : public AbstractPipeline {
    public:
        virtual void addProcessor(AbstractProcessor* processor);

        void addEventListenerToBack(tgt::EventListener* e);
    };

    /* TFGeometry1D */

    %nodefaultctor TFGeometry1D;

    class TFGeometry1D {
    public:
        virtual ~TFGeometry1D();
        static TFGeometry1D* createQuad(const tgt::vec2& interval, const tgt::col4& leftColor, const tgt::vec4& rightColor);
    };

    /* AbstractTransferFunction */

    class AbstractTransferFunction {
    public:
        AbstractTransferFunction(const tgt::svec3& size, const tgt::vec2& intensityDomain = tgt::vec2(0.f, 1.f));
        virtual ~AbstractTransferFunction();

        virtual AbstractTransferFunction* clone() const = 0;
    };

    /* GenericGeometryTransferFunction */

    template<class T>
    class GenericGeometryTransferFunction : public AbstractTransferFunction {
    public:
        GenericGeometryTransferFunction(const tgt::vec3& size, const tgt::vec2& intensityDomain = tgt::vec2(0.f, 1.f));
        virtual ~GenericGeometryTransferFunction();

        void addGeometry(T* geometry);
    };

    /* Geometry1DTransferFunction */

    %template(GenericGeometryTransferFunction_TFGeometry1D) GenericGeometryTransferFunction<TFGeometry1D>;

    class Geometry1DTransferFunction : public GenericGeometryTransferFunction<TFGeometry1D> {
    public:
        Geometry1DTransferFunction(size_t size, const tgt::vec2& intensityDomain = tgt::vec2(0.f, 1.f));
        virtual ~Geometry1DTransferFunction();

        virtual Geometry1DTransferFunction* clone() const;
    };

    /* TransferFunctionProperty */

    class TransferFunctionProperty : public AbstractProperty {
    public:
        TransferFunctionProperty(const std::string& name, const std::string& title, AbstractTransferFunction* tf,
                                 int invalidationLevel = AbstractProcessor::INVALID_RESULT);
        virtual ~TransferFunctionProperty();

        AbstractTransferFunction* getTF();
        void replaceTF(AbstractTransferFunction* tf);
    };

    /* Downcast the return value of VisualizationProcessor::getProperty to appropriate subclass */
    %factory(AbstractProperty* campvis::VisualizationProcessor::getProperty,
             campvis::TransferFunctionProperty, campvis::DataNameProperty, campvis::StringProperty);

    /* VisualizationProcessor */

    class VisualizationProcessor : public AbstractProcessor {
    public:
        explicit VisualizationProcessor(IVec2Property* viewportSizeProp);
        ~VisualizationProcessor();

        AbstractProperty* getProperty(const std::string& name) const;
    };
}

%inline {
static const char* const SOURCE_DIR = CAMPVIS_SOURCE_DIR;

namespace campvis {
    /*
     * Lua pipelines need to access certain protected properties of AbstractPipeline but can't
     * reference them directly as they don't actually inherit from any class. This subclass exposes
     * public getters for the properties in question. LuaPipeline instances can then be cast to
     * ExtendedAutoEvaluationPipeline to give Lua code access to these new methods.
     */
    class ExtendedAutoEvaluationPipeline : public AutoEvaluationPipeline {
    public:
        const IVec2Property& ExtendedAutoEvaluationPipeline::getCanvasSizeProperty() const {
            return _canvasSize;
        }

        const DataNameProperty& ExtendedAutoEvaluationPipeline::getRenderTargetIDProperty() const {
            return _renderTargetID;
        }
    };
}
}

%luacode {
  function campvis.newPipeline (o)
    o = o or {}   -- create object if user does not provide one
    setmetatable(o, {__index = instance})
    return o
  end

  print("Module campvis loaded")
}
