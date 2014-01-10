extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "modules/pipelinefactory.h"
#include "luapipeline.h"


using namespace campvis;


int main()
{
    lua_State* L = luaL_newstate();

    // load the libs
    luaL_openlibs(L);

    // run a Lua script here; true is returned if there were errors
    if (luaL_dofile(L, "script.lua")) {
        const char* msg = lua_tostring(L, -1);

        if (msg == nullptr)
            printf("(error object is not a string)");
        else
            printf("%s", msg);

        lua_pop(L, 1);
    }

    PipelineFactory& pipelineFactory = PipelineFactory::getRef();
    pipelineFactory.registerPipeline<LuaPipeline>([L] (DataContainer* dc) -> AbstractPipeline* {
        return new LuaPipeline(L, dc);
    });

    DataContainer* dc = new DataContainer("Test Data Container");
    AbstractPipeline* p = pipelineFactory.createPipeline("LuaPipeline", dc);
    p->init();
    p->deinit();

    printf("\nI am done with Lua in C++.\n");

    lua_close(L);

    return 0;
}
