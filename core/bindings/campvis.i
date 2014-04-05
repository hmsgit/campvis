%module campvis
%include factory.i
%include std_string.i
%import "ext/tgt/bindings/tgt.i"
%include "ext/sigslot/sigslot.i"
%{
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/imagedata.h"
#include "core/eventhandlers/trackballnavigationeventlistener.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/geometry1dtransferfunction.h"
%}


%inline {
static const char* const SOURCE_DIR = CAMPVIS_SOURCE_DIR;

// Template specialisations and instantiations required to get signals to work in Lua
namespace sigslot {
    template<>
    struct LuaConnectionArgTraits<campvis::AbstractProcessor*> {
        static const char* const typeName;
    };

    const char* const LuaConnectionArgTraits<campvis::AbstractProcessor*>::typeName = "campvis::AbstractProcessor *";
}
}


%template(sigslot_signal1_AbstractProcessor) sigslot::signal1<campvis::AbstractProcessor*>;


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

    /* DataNameProperty */

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

    /* NumericProperty */

    template<typename T>
    class NumericProperty : public GenericProperty<T> {
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

    %template(Ivec2GenericProperty) GenericProperty< tgt::Vector2<int> >;
    %template(IVec2Property) NumericProperty< tgt::Vector2<int> >;
    typedef NumericProperty< tgt::Vector2<int> > IVec2Property;

    template<typename T>
    struct FloatingPointPropertyTraits {};

    template<>
    struct FloatingPointPropertyTraits<float> {
        typedef int DecimalsType;
    };

    %template() FloatingPointPropertyTraits<float>;

    template<typename T>
    class FloatingPointProperty : public NumericProperty<T> {
    public:
        typedef typename FloatingPointPropertyTraits<T>::DecimalsType DecimalsType;

        FloatingPointProperty(
            const std::string& name,
            const std::string& title,
            const T& value,
            const T& minValue,
            const T& maxValue,
            const T& stepValue = T(0.01f),
            const DecimalsType& decimals = DecimalsType(3),
            int invalidationLevel = AbstractProcessor::INVALID_RESULT);

        virtual ~FloatingPointProperty();
    };

    %template(FloatGenericProperty) GenericProperty<float>;
    %template(FloatNumericProperty) NumericProperty<float>;
    %template(FloatProperty) FloatingPointProperty<float>;
    typedef FloatingPointProperty< float > FloatProperty;

    /* CameraProperty */

    %template(GenericProperty_Camera) GenericProperty<tgt::Camera>;

    class CameraProperty : public GenericProperty<tgt::Camera> {
    public:
        CameraProperty(const std::string& name, const std::string& title, tgt::Camera cam = tgt::Camera(),
                       int invalidationLevel = AbstractProcessor::INVALID_RESULT);
        virtual ~CameraProperty();
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

    /* IHasWorldBounds */

    class IHasWorldBounds {
    public:
        IHasWorldBounds();
        virtual ~IHasWorldBounds();

        virtual tgt::Bounds getWorldBounds() const = 0;
    };

    /* AbstractData */

    class AbstractData {
    public:
        AbstractData();
        virtual ~AbstractData();

        virtual AbstractData* clone() const = 0;
    };

    /* ImageData */

    class ImageData : public AbstractData, public IHasWorldBounds {
    public:
        ImageData(size_t dimensionality, const tgt::svec3& size, size_t numChannels);
        virtual ~ImageData();

        virtual ImageData* clone() const;
        virtual tgt::Bounds getWorldBounds() const;
    };

    /* Downcast the return value of DataHandle::getData to appropriate subclass */
    %factory(AbstractData* campvis::DataHandle::getData, campvis::ImageData);

    /* DataHandle */

    class DataHandle {
    public:
        explicit DataHandle(AbstractData* data = 0);
        DataHandle(const DataHandle& rhs);
        DataHandle& operator=(const DataHandle& rhs);
        virtual ~DataHandle();

        const AbstractData* getData() const;
    };

    /* DataContainer */

    class DataContainer {
    public:
        DataContainer(const std::string& name);
        ~DataContainer();

        DataHandle addData(const std::string& name, AbstractData* data);
        void addDataHandle(const std::string& name, const DataHandle& dh);
        bool hasData(const std::string& name) const;
        DataHandle getData(const std::string& name) const;
        void removeData(const std::string& name);
        void clear();
    };

    /* Downcast the return value of HasPropertyCollection::getProperty to appropriate subclass */
    %factory(AbstractProperty* campvis::HasPropertyCollection::getProperty,
             campvis::FloatProperty, campvis::IVec2Property, campvis::TransferFunctionProperty,
             campvis::DataNameProperty, campvis::StringProperty, campvis::CameraProperty);

    /* Downcast the return value of HasPropertyCollection::getNestedProperty to appropriate subclass */
    %factory(AbstractProperty* campvis::HasPropertyCollection::getNestedProperty,
             campvis::FloatProperty, campvis::IVec2Property, campvis::TransferFunctionProperty,
             campvis::DataNameProperty, campvis::StringProperty, campvis::CameraProperty);

    /* HasPropertyCollection */

    class HasPropertyCollection  {
    public:
        HasPropertyCollection();
        virtual ~HasPropertyCollection() = 0;

        void addProperty(AbstractProperty* prop);
        void removeProperty(AbstractProperty* prop);
        AbstractProperty* getProperty(const std::string& name) const;
        AbstractProperty* getNestedProperty(const std::string& name) const;
    };

    /* AbstractProcessor */

    class AbstractProcessor : public HasPropertyCollection {
    public:
        enum InvalidationLevel {
            VALID               = 0,
            INVALID_RESULT      = 1 << 0,
            INVALID_SHADER      = 1 << 1,
            INVALID_PROPERTIES  = 1 << 2,
            FIRST_FREE_TO_USE_INVALIDATION_LEVEL = 1 << 3
        };

        const std::string getName() const = 0;

        %immutable;
        sigslot::signal1<AbstractProcessor*> s_validated;
        %mutable;
    };

    /* AbstractPipeline */

    class AbstractPipeline : public HasPropertyCollection {
    public:
        AbstractPipeline(DataContainer* dc);
        virtual ~AbstractPipeline();

        virtual const std::string getName() const = 0;

        const DataContainer& getDataContainer() const;
        DataContainer& getDataContainer();
    };

    /* AutoEvaluationPipeline */

    class AutoEvaluationPipeline : public AbstractPipeline {
    public:
        virtual void addProcessor(AbstractProcessor* processor);

        void addEventListenerToBack(tgt::EventListener* e);
    };

    /* VisualizationProcessor */

    class VisualizationProcessor : public AbstractProcessor {
    public:
        explicit VisualizationProcessor(IVec2Property* viewportSizeProp);
        ~VisualizationProcessor();
    };

    /* TrackballNavigationEventListener */

    class TrackballNavigationEventListener : public tgt::EventListener {
    public:
        TrackballNavigationEventListener(CameraProperty* cameraProperty, IVec2Property* viewportSizeProp);
        virtual ~TrackballNavigationEventListener();

        void addLqModeProcessor(VisualizationProcessor* vp);
        void removeLqModeProcessor(VisualizationProcessor* vp);

        void reinitializeCamera(const IHasWorldBounds* hwb);
        void reinitializeCamera(const tgt::Bounds& worldBounds);
    };
}


%luacode {
  function campvis.newPipeline (name, o)
    if not name then
      error("A name must be provided when creating a new pipeline!")
    end

    o = o or {}   -- create object if user does not provide one
    setmetatable(o, {__index = instance})
    return o
  end

  print("Module campvis loaded")
}
