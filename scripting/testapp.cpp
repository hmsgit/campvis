#include "luapipeline.h"


using namespace campvis;


int main()
{
    DataContainer* dc = new DataContainer("Test Data Container");
    AbstractPipeline* p = new LuaPipeline("Test Lua Pipeline",
                                          CAMPVIS_SOURCE_DIR "/modules/preprocessing/pipelines/resamplingdemo.lua", dc);

    p->init();
    p->deinit();

    return 0;
}
