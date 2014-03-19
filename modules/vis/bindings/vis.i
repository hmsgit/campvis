%module vis
%include std_string.i
%import "core/bindings/campvis.i"
%{
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/vis/processors/volumeexplorer.h"
%}


namespace campvis {

    class VolumeExplorer : public VisualizationProcessor, public tgt::EventListener {
    public:
        VolumeExplorer(campvis::IVec2Property* viewportSizeProp);
        ~VolumeExplorer();

        const std::string getName() const;

        %immutable;
        campvis::DataNameProperty p_inputVolume;
        campvis::DataNameProperty p_outputImage;
        %mutable;
    };
}
