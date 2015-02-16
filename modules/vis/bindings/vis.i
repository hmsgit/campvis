%module vis
%include std_string.i
%include "core/bindings/campvis.i"
%{
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/vis/processors/volumeexplorer.h"
#include "modules/vis/processors/volumerenderer.h"

#include "modules/processorfactory.h"
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

    /* ProcessorFactory */

    class ProcessorFactory {
    public:
        static ProcessorFactory& getRef();
        std::vector<std::string> getRegisteredProcessors() const;
        AbstractProcessor* createProcessor(const std::string& id, IVec2Property* viewPortSizeProp = 0) const;
    };
    
    /* Down casting or super classes.
     * Down casting follows the order of declaration.
     * Declare the classes as child first according to the class hierarchy.
     */

    /* Downcast the return value of HasPropertyCollection::getProperty to appropriate subclass */
    %factory(AbstractProperty* campvis::ProcessorFactory::createProcessor,
             campvis::SimpleRaycaster, campvis::OptimizedRaycaster, campvis::DRRRaycaster, campvis::ContextPreservingRaycaster, campvis::AdvOptimizedRaycaster,
             campvis::SliceExtractor, SliceRenderer2D, SliceRenderer3D,
             campvis::VolumeExplorer, campvis::VolumeRenderer, 
             campvis::MicroscopyImageSegmentation, 
             campvis::GlGaussianFilter, campvis::GlGradientVolumeGenerator, campvis::GlImageCrop, campvis::GlImageResampler, campvis::GlIntensityQuantizer, campvis::GlMorphologyFilter, 
             campvis::GlSignalToNoiseRatioFilter, campvis::GlVesselnessFilter, campvis::TensorGlyphRenderer, campvis::ParticleFlowRenderer, 
             campvis::VectorFieldRenderer,
             campvis::QuadView, campvis::RenderTargetCompositor, campvis::VirtualMirrorCombine, 
             campvis::DepthDarkening, campvis::EEPGenerator, campvis::GeometryRenderer, campvis::MprRenderer);

}

%luacode {
  print("Module campvis-vis loaded")
}
