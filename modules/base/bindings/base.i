%module base
%include std_string.i
%import "ext/cgt/bindings/cgt.i"


%import "core/bindings/campvis.i"

%{
#include "core/properties/allproperties.h"
#include "core/pipeline/abstractworkflow.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/visualizationprocessor.h"
#include "modules/base/processors/lightsourceprovider.h"
#include "modules/base/processors/trackballcameraprovider.h"
%}


namespace campvis {

    class LightSourceProvider : public AbstractProcessor {
    public:
        LightSourceProvider();
        virtual ~LightSourceProvider();

        const std::string getName() const;
    };
    
    class TrackballCameraProvider : public AbstractProcessor, public cgt::EventListener {
    public:
        TrackballCameraProvider(IVec2Property* canvasSize);
        virtual ~TrackballCameraProvider();

        const std::string getName() const;
        
        void addLqModeProcessor(VisualizationProcessor* vp);
        void removeLqModeProcessor(VisualizationProcessor* vp);
        void reinitializeCamera(const cgt::Bounds& worldBounds);

        %immutable;
        campvis::DataNameProperty p_image;
        %mutable;
    };
}

%luacode {
  print("Module campvis-base loaded")
}
