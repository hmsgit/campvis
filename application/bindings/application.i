%module application

%include factory.i
%include std_pair.i
%include std_string.i
%include std_vector.i

%import "core/bindings/campvis.i"

%{
#include "application/campvisapplication.h"
#include "core/properties/allproperties.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/abstractworkflow.h"
#include "core/pipeline/autoevaluationpipeline.h"
%}

namespace campvis {
    class CampVisApplication {

    public:
        CampVisApplication(int& argc, char** argv);
        ~CampVisApplication();

        void init();
        void deinit();
        int run();
        
        %apply SWIGTYPE *DISOWN {AbstractPipeline* pipeline};
        void addPipeline(AbstractPipeline* pipeline);
        DataContainer* createAndAddDataContainer(const std::string& name);
        %clear AbstractPipeline* pipeline;

        void rebuildAllShadersFromFiles();
        void setPipelineVisibility(AbstractPipeline* pipeline, bool visibility);

        /// Signal emitted when the collection of pipelines has changed.
        sigslot::signal0 s_PipelinesChanged;
        /// Signal emitted when the collection of DataContainers has changed.
        sigslot::signal0 s_DataContainersChanged;

    };

}

%luacode {
  print("Module campvis-application loaded")
}

