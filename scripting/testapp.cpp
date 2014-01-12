#include "modules/pipelinefactory.h"
#include "luapipeline.h"


using namespace campvis;


int main()
{
    PipelineFactory& pipelineFactory = PipelineFactory::getRef();
    pipelineFactory.registerPipeline<LuaPipeline>([] (DataContainer* dc) -> AbstractPipeline* {
        return new LuaPipeline("script.lua", dc);
    });

    DataContainer* dc = new DataContainer("Test Data Container");
    AbstractPipeline* p = pipelineFactory.createPipeline("LuaPipeline", dc);
    p->init();
    p->deinit();

    return 0;
}
