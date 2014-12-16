%module cvio
%include std_string.i
%include "core/bindings/campvis.i"
%{
#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/io/processors/mhdimagewriter.h"
%}


namespace campvis {

    class AbstractImageReader : public AbstractProcessor {
    public:
        AbstractImageReader();
        ~AbstractImageReader();

        %immutable;
        campvis::StringProperty p_url;
        campvis::DataNameProperty p_targetImageID;
        %mutable;
    };

    class MhdImageReader : public AbstractImageReader {
    public:
        MhdImageReader();
        ~MhdImageReader();

        const std::string getName() const;
    };

    class MhdImageWriter : public AbstractProcessor {
    public:
        MhdImageWriter();
        virtual ~MhdImageWriter();

        virtual const std::string getName() const;

        %immutable;
        DataNameProperty p_inputImage;
        StringProperty p_fileName;
        ButtonProperty p_saveFile;
        %mutable;
    };
}

%luacode {
  print("Module campvis-io loaded")
}

