%module base
%include std_string.i
%import "core/bindings/campvis.i"
%{
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/visualizationprocessor.h"
#include "modules/base/processors/lightsourceprovider.h"
%}


namespace campvis {

    class LightSourceProvider : public AbstractProcessor {
    public:
        LightSourceProvider();
        virtual ~LightSourceProvider();

        const std::string getName() const;
    };
}
