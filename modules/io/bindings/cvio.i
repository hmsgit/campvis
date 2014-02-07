%module cvio
%include std_string.i
%import "core/bindings/campvis.i"
%{
#include "core/pipeline/visualizationprocessor.h"
#include "modules/io/processors/mhdimagereader.h"
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
}
