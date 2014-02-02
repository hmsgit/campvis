#ifndef SCRIPTEDPIPELINEREGISTRAR_H__
#define SCRIPTEDPIPELINEREGISTRAR_H__

#include "luapipeline.h"
#include "modules/pipelinefactory.h"


namespace campvis {

    static const size_t _factoryId = PipelineFactory::getRef().registerPipeline<LuaPipeline>([] (DataContainer* dc) -> AbstractPipeline* {
        return new campvis::LuaPipeline(CAMPVIS_SOURCE_DIR "/scripting/script.lua", dc);
    });
}

#endif // SCRIPTEDPIPELINEREGISTRAR_H__
