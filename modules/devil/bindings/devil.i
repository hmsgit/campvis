%module devil

%include std_string.i
%import "core/bindings/campvis.i"

%{
#include "core/properties/allproperties.h"
#include "core/pipeline/abstractworkflow.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/visualizationprocessor.h"
#include "modules/devil/processors/devilimagereader.h"
#include "modules/devil/processors/devilimagewriter.h"
%}


namespace campvis {

    class DevilImageWriter : public AbstractProcessor {
    public:
        DevilImageWriter();
        virtual ~DevilImageWriter();

        virtual const std::string getName() const;

        %immutable;
        DataNameProperty p_inputImage;      ///< image ID for image to write
        StringProperty p_url;               ///< URL for file to write
        BoolProperty p_writeDepthImage;     ///< Flag whether to save also depth image
        %mutable;
    };
}

%luacode {
  print("Module campvis-devil loaded")
}

