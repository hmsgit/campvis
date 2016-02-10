%module campvis

%include factory.i
%include std_pair.i
%include std_string.i
%include std_vector.i

%import "ext/cgt/bindings/cgt.i"
%include "ext/sigslot/sigslot.i"

%{
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/dataseries.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/properties/allproperties.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/abstractworkflow.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/pipelinefactory.h"
#include "core/pipeline/processorfactory.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/tfgeometry2d.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry2dtransferfunction.h"
%}


%inline {
static const char* const SOURCE_DIR = CAMPVIS_SOURCE_DIR;

// Template specialisations and instantiations required to get signals to work in Lua
namespace sigslot {
    template<>
    struct LuaConnectionArgTraits<campvis::AbstractProcessor*> { static const char* const typeName; };
    const char* const LuaConnectionArgTraits<campvis::AbstractProcessor*>::typeName = "campvis::AbstractProcessor *";

    template<>
    struct LuaConnectionArgTraits<campvis::DataHandle> { static const char* const typeName; };
    const char* const LuaConnectionArgTraits<campvis::DataHandle>::typeName = "campvis::DataHandle *";

    template<>
    struct LuaConnectionArgTraits<std::string> { static const char* const typeName; };
    const char* const LuaConnectionArgTraits<std::string>::typeName = "std::string *";
}
}


%template(sigslot_signal1_AbstractProcessor) sigslot::signal1<campvis::AbstractProcessor*>;
%template(sigslot_signal2_string_DataHandle) sigslot::signal2<std::string, campvis::DataHandle>;

%template(PairStringDataHandle) std::pair<std::string, campvis::DataHandle>;
%template(VectorOfPairStringDataHandle) std::vector< std::pair< std::string, campvis::DataHandle> >;
%template(VectorOfString) std::vector< std::string >;

namespace campvis {

    class AbstractProperty {
    public:
        AbstractProperty(const std::string& name, const std::string& title);
        virtual ~AbstractProperty();

        virtual void init();
        virtual void deinit();

        const std::string& getName() const;
        const std::string& getTitle() const;

        bool isVisible() const;
        void setVisible(bool isVisible);

        virtual void addSharedProperty(AbstractProperty* prop);
    };

    class ButtonProperty : public AbstractProperty {
    public:
        ButtonProperty(const std::string& name, const std::string& title);
        virtual ~ButtonProperty();

        void click();

        sigslot::signal0 s_clicked;
    };

    template<typename T>
    class GenericProperty : public AbstractProperty {
    public:
        GenericProperty(const std::string& name, const std::string& title, const T& value);
        virtual ~GenericProperty();

        const T& getValue() const;
        virtual void setValue(const T& value);
    };

    %template(BoolProperty) GenericProperty<bool>;
    typedef GenericProperty<bool> BoolProperty;

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
                         DataAccessInfo access);
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
            const T& stepValue = T(1));

        virtual ~NumericProperty();
    };

    
    %template(IntGenericProperty) GenericProperty< int >;
    %template(IntProperty) NumericProperty< int >;
    typedef NumericProperty< int > IntProperty;

    %template(Ivec2GenericProperty) GenericProperty< cgt::Vector2<int> >;
    %template(IVec2Property) NumericProperty< cgt::Vector2<int> >;
    typedef NumericProperty< cgt::Vector2<int> > IVec2Property;

    %template(Ivec3GenericProperty) GenericProperty< cgt::Vector3<int> >;
    %template(IVec3Property) NumericProperty< cgt::Vector3<int> >;
    typedef NumericProperty< cgt::Vector3<int> > IVec3Property;

    %template(Ivec4GenericProperty) GenericProperty< cgt::Vector4<int> >;
    %template(IVec4Property) NumericProperty< cgt::Vector4<int> >;
    typedef NumericProperty< cgt::Vector4<int> > IVec4Property;



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
            const DecimalsType& decimals = DecimalsType(3));

        virtual ~FloatingPointProperty();
    };

    %template(FloatGenericProperty) GenericProperty<float>;
    %template(FloatNumericProperty) NumericProperty<float>;
    %template(FloatProperty) FloatingPointProperty<float>;
    typedef FloatingPointProperty< float > FloatProperty;

    %template(Vec2GenericProperty) GenericProperty< cgt::Vector2<float> >;
    %template(Vec2NumericProperty) NumericProperty< cgt::Vector2<float> >;
    %template(Vec2Property) FloatingPointProperty< cgt::Vector2<float> >;
    typedef FloatingPointProperty< cgt::Vector2<float> > Vec2Property;

    %template(Vec3GenericProperty) GenericProperty< cgt::Vector3<float> >;
    %template(Vec3NumericProperty) NumericProperty< cgt::Vector3<float> >;
    %template(Vec3Property) FloatingPointProperty< cgt::Vector3<float> >;
    typedef FloatingPointProperty< cgt::Vector3<float> > Vec3Property;

    %template(Vec4GenericProperty) GenericProperty< cgt::Vector4<float> >;
    %template(Vec4NumericProperty) NumericProperty< cgt::Vector4<float> >;
    %template(Vec4Property) FloatingPointProperty< cgt::Vector4<float> >;
    typedef FloatingPointProperty< cgt::Vector4<float> > Vec4Property;

    /* OptionProperty */
        
    class AbstractOptionProperty : public IntProperty {
    public:
        AbstractOptionProperty(const std::string& name, const std::string& title);
        virtual ~AbstractOptionProperty();
        virtual const std::string& getOptionId() = 0;
        virtual void selectById(const std::string& id) = 0;
    };

    template<typename T>
    class GenericOptionProperty : public AbstractOptionProperty {
    public:		
        GenericOptionProperty(
            const std::string& name, 
            const std::string& title, 
            const GenericOption<T>* options,
            int count);		
        virtual ~GenericOptionProperty();		
        const std::string& getOptionId();
        void selectById(const std::string& id);
    };

    /* TFGeometry1D */

    %nodefaultctor TFGeometry1D;

    class TFGeometry1D {
    public:
        virtual ~TFGeometry1D();
        static TFGeometry1D* createQuad(const cgt::vec2& interval, const cgt::col4& leftColor, const cgt::col4& rightColor);
    
        void addKeyPoint(float position, float alpha);
        void addKeyPoint(float position, const cgt::col4& color);
    };

    /* TFGeometry2D */

    %nodefaultctor TFGeometry2D;

    class TFGeometry2D {
    public:
        virtual ~TFGeometry2D();
        static TFGeometry2D* createQuad(const cgt::vec2& ll, const cgt::vec2& ur, const cgt::col4& color);
    };

    /* AbstractTransferFunction */

    class AbstractTransferFunction {
    public:
        AbstractTransferFunction(const cgt::svec3& size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));
        virtual ~AbstractTransferFunction();

        virtual AbstractTransferFunction* clone() const = 0;
    };

    /* SimpleTransferFunction */
    class SimpleTransferFunction : public AbstractTransferFunction {
    public: 
        SimpleTransferFunction(size_t size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));
        virtual ~SimpleTransferFunction();
        virtual SimpleTransferFunction* clone() const;
        void setLeftColor(const cgt::col4& color);
        void setRightColor(const cgt::col4& color);

    };

    /* GenericGeometryTransferFunction */

    template<class T>
    class GenericGeometryTransferFunction : public AbstractTransferFunction {
    public:
        GenericGeometryTransferFunction(const cgt::vec3& size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));
        virtual ~GenericGeometryTransferFunction();

        %apply SWIGTYPE *DISOWN {T* geometry};
        void addGeometry(T* geometry);
        %clear T* geometry;
    };
    
    /* Geometry1DTransferFunction */

    %template(GenericGeometryTransferFunction_TFGeometry1D) GenericGeometryTransferFunction<TFGeometry1D>;

    class Geometry1DTransferFunction : public GenericGeometryTransferFunction<TFGeometry1D> {
    public:
        Geometry1DTransferFunction(size_t size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));
        virtual ~Geometry1DTransferFunction();

        virtual Geometry1DTransferFunction* clone() const;
    };

    /* Geometry2DTransferFunction */

    %template(GenericGeometryTransferFunction_TFGeometry2D) GenericGeometryTransferFunction<TFGeometry2D>;

    class Geometry2DTransferFunction : public GenericGeometryTransferFunction<TFGeometry2D> {
    public:
        Geometry2DTransferFunction(const cgt::svec2& size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));
        virtual ~Geometry2DTransferFunction();

        virtual Geometry2DTransferFunction* clone() const;
    };

    /* TransferFunctionProperty */

    class TransferFunctionProperty : public AbstractProperty {
    public:
        %apply SWIGTYPE *DISOWN {AbstractTransferFunction* tf};
        TransferFunctionProperty(const std::string& name, const std::string& title, AbstractTransferFunction* tf);
        virtual ~TransferFunctionProperty();

        AbstractTransferFunction* getTF();
        void replaceTF(AbstractTransferFunction* tf);
        %clear AbstractTransferFunction* tf;
    };

    /* IHasWorldBounds */

    class IHasWorldBounds {
    public:
        IHasWorldBounds();
        virtual ~IHasWorldBounds();

        virtual cgt::Bounds getWorldBounds() const = 0;
    };

    /* AbstractData */

    class AbstractData {
    public:
        AbstractData();
        virtual ~AbstractData();

        virtual size_t getLocalMemoryFootprint() const = 0;
        virtual size_t getVideoMemoryFootprint() const = 0;
        virtual AbstractData* clone() const = 0;
    };
    
    /* DataHandle */

    /* Downcast the return value of DataHandle::getData to appropriate subclass */
    %factory(const AbstractData* campvis::DataHandle::getData, const campvis::ImageData);

    %apply SWIGTYPE *DISOWN {AbstractData* data};
    class DataHandle {
    public:
        explicit DataHandle(AbstractData* data = 0);
        DataHandle(const DataHandle& rhs);
        DataHandle& operator=(const DataHandle& rhs);
        virtual ~DataHandle();

        const AbstractData* getData() const;
    };

    /* DataSeries */

    class DataSeries : public AbstractData {
    public:
        DataSeries();
        virtual ~DataSeries();
        virtual DataSeries* clone() const;
        
        void addData(AbstractData* data);
        void addData(DataHandle dh);

        size_t getNumDatas() const;
        DataHandle getData(size_t index) const;

    protected:
        std::vector<DataHandle> _data;    ///< the images of this series
    };

    /* ImageData */

    class ImageData : public AbstractData, public IHasWorldBounds {
    public:
        ImageData(size_t dimensionality, const cgt::svec3& size, size_t numChannels);
        virtual ~ImageData();

        virtual AbstractData* clone() const;
        virtual size_t getLocalMemoryFootprint() const;
        virtual size_t getVideoMemoryFootprint() const;
        virtual std::string getTypeAsString() const;

        size_t getDimensionality() const;
        const cgt::svec3& getSize() const;
        size_t getNumChannels() const;
        size_t getNumElements() const;

        virtual cgt::Bounds getWorldBounds() const;
        cgt::Bounds getWorldBounds(const cgt::svec3& llf, const cgt::svec3& urb) const;

        size_t positionToIndex(const cgt::svec3& position) const;
        cgt::svec3 indexToPosition(size_t index) const;

        template<typename T>
        const T* getRepresentation(bool performConversion = true) const;
    };

    %template(getRepresentationLocal) ImageData::getRepresentation<campvis::ImageRepresentationLocal>;

    /*struct WeaklyTypedPointer {
        enum BaseType {
            UINT8,      ///< unsigned 8 bit integer
            INT8,       ///< signed 8 bit integer
            UINT16,     ///< unsigned 16 bit integer
            INT16,      ///< signed 16 bit integer
            UINT32,     ///< unsigned 32 bit integer
            INT32,      ///< signed 32 bit integer
            FLOAT      ///< float
        };
        static size_t numBytes(BaseType bt, size_t numChannels = 1);

        WeaklyTypedPointer(BaseType pt, size_t numChannels, void* ptr);
        WeaklyTypedPointer();
        virtual ~WeaklyTypedPointer();

        bool operator==(const WeaklyTypedPointer& rhs) const;

        size_t getNumBytesPerElement() const;
        GLint getGlFormat() const;
        GLenum getGlDataType() const;
        GLint getGlInternalFormat() const;
        bool isInteger() const;
        bool isSigned() const;

        BaseType _baseType;         ///< Base data type of the pointer
        size_t _numChannels;        ///< Number of channels, must be in [1, 4]!
        void* _pointer;             ///< Pointer to the data
    };*/

    /* AbstractImageRepresentation */
    %nodefaultctor AbstractImageRepresentation;
    class AbstractImageRepresentation {
    public:
        virtual ~AbstractImageRepresentation();

        const ImageData* getParent() const;

        size_t getDimensionality() const;
        const cgt::svec3& getSize() const;
        size_t getNumElements() const;

    private:
        explicit AbstractImageRepresentation(const AbstractImageRepresentation& rhs);
        AbstractImageRepresentation& operator=(const AbstractImageRepresentation& rhs);
    };


    /* ImageRepresentationLocal */
    %nodefaultctor ImageRepresentationLocal;
    class ImageRepresentationLocal : public AbstractImageRepresentation {
    public:
        virtual ~ImageRepresentationLocal();

        //static ImageRepresentationLocal* create(const ImageData* parent, WeaklyTypedPointer wtp);

        //virtual const WeaklyTypedPointer getWeaklyTypedPointer() const = 0;
        virtual float getElementNormalized(size_t index, size_t channel) const = 0;

        virtual float getElementNormalized(const cgt::svec3& position, size_t channel) const = 0;
        virtual float getElementNormalizedLinear(const cgt::vec3& position, size_t channel) const = 0;

        virtual void setElementNormalized(size_t index, size_t channel, float value) = 0;
        virtual void setElementNormalized(const cgt::svec3& position, size_t channel, float value) = 0;
        
    protected:
        //ImageRepresentationLocal(ImageData* parent, WeaklyTypedPointer::BaseType baseType);

    private:
        explicit ImageRepresentationLocal(const ImageRepresentationLocal& rhs);
        ImageRepresentationLocal& operator=(const ImageRepresentationLocal& rhs);
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

        std::vector< std::pair< std::string, DataHandle> > getDataHandlesCopy() const;

        %immutable;
        sigslot::signal0 s_changed;
        sigslot::signal2<std::string, DataHandle> s_dataAdded;
        %mutable;
    };

    %clear AbstractData* data;
    
    /* Down casting or super classes.
     * Down casting follows the order of declaration.
     * Declare the classes as child first according to the class hierarchy.
     */

    /* Downcast the return value of HasPropertyCollection::getProperty to appropriate subclass */
    %factory(AbstractProperty* campvis::HasPropertyCollection::getProperty,
             campvis::AbstractOptionProperty, campvis::IntProperty, campvis::IVec2Property, campvis::IVec3Property, campvis::IVec4Property,
             campvis::FloatProperty, campvis::Vec2Property, campvis::Vec3Property, campvis::Vec4Property,
             campvis::TransferFunctionProperty,
             campvis::DataNameProperty, campvis::StringProperty, campvis::ButtonProperty, campvis::BoolProperty);

    /* Downcast the return value of HasPropertyCollection::getNestedProperty to appropriate subclass */
    %factory(AbstractProperty* campvis::HasPropertyCollection::getNestedProperty,
             campvis::AbstractOptionProperty, campvis::IntProperty, campvis::IVec2Property, campvis::IVec3Property, campvis::IVec4Property,
             campvis::FloatProperty, campvis::Vec2Property, campvis::Vec3Property, campvis::Vec4Property,
             campvis::TransferFunctionProperty,
             campvis::DataNameProperty, campvis::StringProperty, campvis::ButtonProperty, campvis::BoolProperty);

    /* HasPropertyCollection */

    class HasPropertyCollection  {
    public:
        HasPropertyCollection();
        virtual ~HasPropertyCollection() = 0;

        virtual void addProperty(AbstractProperty& prop);
        void removeProperty(AbstractProperty& prop);
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
        void addProperty(AbstractProperty& prop, int invalidationLevel);
        void setPropertyInvalidationLevel(AbstractProperty& prop, int invalidationLevel);

        void process(DataContainer& data);

        %immutable;
        sigslot::signal1<AbstractProcessor*> s_validated;
        sigslot::signal1<AbstractProcessor*> s_invalidated;
        %mutable;
    };

    /* AbstractPipeline */

    class AbstractPipeline : public HasPropertyCollection {
    public:
        AbstractPipeline(DataContainer& dc);
        virtual ~AbstractPipeline();

        virtual std::string getName() const = 0;

        const DataContainer& getDataContainer() const;
        DataContainer& getDataContainer();
        
        virtual void addProcessor(AbstractProcessor* processor);
        virtual void executePipeline() = 0;
        AbstractProcessor* getProcessor(const std::string& name) const;
        AbstractProcessor* getProcessor(size_t index) const;

        %immutable;
        sigslot::signal0 s_init;
        sigslot::signal0 s_deinit;
        %mutable;
    };

    /* AutoEvaluationPipeline */

    class AutoEvaluationPipeline : public AbstractPipeline {
    public:
        AutoEvaluationPipeline(DataContainer& dataContainer, const std::string& pipelineName);
        virtual ~AutoEvaluationPipeline();
        
        std::string getName() const;
        virtual void addProcessor(AbstractProcessor* processor);

        void addEventListenerToBack(cgt::EventListener* e);
        virtual void executePipeline();
    };

    class AbstractWorkflow : public HasPropertyCollection {
    public:
        AbstractWorkflow(const std::string& title);
        virtual ~AbstractWorkflow();

        virtual std::vector<AbstractPipeline*> getPipelines() = 0;
        virtual const std::string getName() const = 0;

        virtual void init();
        virtual void deinit();

        DataContainer* getDataContainer();

        virtual bool isStageAvailable(int stage) const;
        int getCurrentStageId() const;
        void setCurrentStage(int stage);

        %immutable;
        sigslot::signal2<int, int> s_stageChanged;
        sigslot::signal0 s_stageAvailabilityChanged;
        %mutable;
    };


    /* VisualizationProcessor */

    class VisualizationProcessor : public AbstractProcessor {
    public:
        explicit VisualizationProcessor(IVec2Property* viewportSizeProp);
        ~VisualizationProcessor();
    };



    /* PipelineFactory and ProcessorFactory */

    class PipelineFactory {
    public:
        static PipelineFactory& getRef();

        std::vector<std::string> getRegisteredPipelines() const;
        AbstractPipeline* createPipeline(const std::string& id, DataContainer& dc) const;
        AbstractWorkflow* createWorkflow(const std::string& id) const;
    };

    class ProcessorFactory {
    public:
        static ProcessorFactory& getRef();
    
        std::vector<std::string> getRegisteredProcessors() const;
        std::vector<std::string> getRegisteredRaycastingProcessors() const;
        AbstractProcessor* createProcessor(const std::string& id, IVec2Property* viewPortSizeProp = nullptr) const;
    };
}

%luacode {
  print("Module campvis-core loaded")
}
