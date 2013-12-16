%module vis
%include "std_string.i"
%include "campvis.i"
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

    class MhdImageReader {
    public:
        MhdImageReader();
        ~MhdImageReader();

        %immutable;
        campvis::StringProperty p_url;
        campvis::DataNameProperty p_targetImageID;
        %mutable;
    };

    class GlImageResampler {
    public:
        GlImageResampler(campvis::IVec2Property* viewportSizeProp);
        ~GlImageResampler();

        %immutable;
        campvis::DataNameProperty p_outputImage;
        %mutable;
    };

    class VolumeExplorer {
    public:
        VolumeExplorer(campvis::IVec2Property* viewportSizeProp);
        ~VolumeExplorer();

        %immutable;
        campvis::DataNameProperty p_outputImage;
        %mutable;
    };
}
