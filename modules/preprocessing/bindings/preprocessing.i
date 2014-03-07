%module preprocessing
%include std_string.i
%import "core/bindings/campvis.i"
%{
#include "modules/preprocessing/processors/glimageresampler.h"
%}

namespace campvis {

    class GlImageResampler : public VisualizationProcessor {
    public:
        GlImageResampler(campvis::IVec2Property* viewportSizeProp);
        ~GlImageResampler();

        const std::string getName() const;

        %immutable;
        campvis::DataNameProperty p_inputImage;
        campvis::DataNameProperty p_outputImage;
        campvis::FloatProperty p_resampleScale;
        %mutable;
    };
}
