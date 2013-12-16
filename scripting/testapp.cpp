extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

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

    printf("\nI am done with Lua in C++.\n");

    lua_close(L);

    return 0;
}
