%module vis
%include std_string.i
%include campvis.i
%{
#include "modules/io/processors/mhdimagereader.h"
#include "modules/preprocessing/processors/glimageresampler.h"
#include "modules/vis/processors/volumeexplorer.h"
%}

%inline %{
int first(tgt::ivec2 &v) {
  return v[0];
}
%}

namespace campvis {

    class AbstractImageReader : public AbstractProcessor {
    public:
        AbstractImageReader();
        ~AbstractImageReader();
    };

    class MhdImageReader : public AbstractImageReader {
    public:
        MhdImageReader();
        ~MhdImageReader();

        const std::string getName() const;

        %immutable;
        campvis::StringProperty p_url;
        campvis::DataNameProperty p_targetImageID;
        %mutable;
    };

    class GlImageResampler : public VisualizationProcessor {
    public:
        GlImageResampler(campvis::IVec2Property* viewportSizeProp);
        ~GlImageResampler();

        const std::string getName() const;

        %immutable;
        campvis::DataNameProperty p_inputImage;
        campvis::DataNameProperty p_outputImage;
        %mutable;
    };

    class VolumeExplorer : public VisualizationProcessor {
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
