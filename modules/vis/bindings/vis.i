%module vis
%include std_string.i
%include "core/bindings/campvis.i"
%{
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/vis/processors/volumeexplorer.h"
#include "modules/vis/processors/volumerenderer.h"

%}


namespace campvis {

    /*
     * As of version 2.0.12, SWIG still has trouble when a default argument is initialised using the
     * `new` keyword. The `default` typemap provides a reasonable workaround.
     */
    %typemap(default) campvis::SliceRenderProcessor* slicerenderer {
       $1 = new campvis::SliceExtractor(0);
    }
    %typemap(default) campvis::RaycastingProcessor* raycaster {
       $1 = new campvis::SimpleRaycaster(0);
    }

    class VolumeExplorer : public VisualizationProcessor, public cgt::EventListener {
    public:
        VolumeExplorer(campvis::IVec2Property* viewportSizeProp);
        ~VolumeExplorer();

        const std::string getName() const;

        %immutable;
        campvis::DataNameProperty p_inputVolume;
        campvis::DataNameProperty p_outputImage;
        %mutable;
    };

    class VolumeRenderer : public VisualizationProcessor {
    public:
        VolumeRenderer(campvis::IVec2Property* viewportSizeProp, campvis::RaycastingProcessor* raycaster);
        virtual ~VolumeRenderer();

        const std::string getName() const;

        %immutable;
        campvis::DataNameProperty p_inputVolume;
        campvis::DataNameProperty p_outputImage;
        %mutable;
    };

}

%luacode {
  print("Module campvis-vis loaded")
}
