%module microscopysegmentation
%include std_string.i
%import "core/bindings/campvis.i"

%{
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/microscopysegmentation/processors/microscopyimagesegmentation.h"
%}


namespace campvis {

    class ContourObject {
    public:
        ContourObject(std::string name);
        virtual ~ContourObject();
        ContourObject* operator=(const campvis::ContourObject& rhs);

    };
    

    class MicroscopyImageSegmentation : public VisualizationProcessor, public HasProcessorDecorators, public cgt::EventListener {
    public:
        MicroscopyImageSegmentation(campvis::IVec2Property* viewportSizeProp, campvis::RaycastingProcessor* raycaster);
        virtual ~MicroscopyImageSegmentation();

        const std::string getName() const;
        
        %immutable;
        campvis::DataNameProperty p_inputVolume;
        campvis::DataNameProperty p_outputImage;

        %mutable;
        
    };

}
