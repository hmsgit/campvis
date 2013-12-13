extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

int main()
{
    int s = 0;

    lua_State* L = luaL_newstate();

    // load the libs
    luaL_openlibs(L);

    // run a Lua script here
    luaL_dofile(L, "script.lua");

    printf("\nI am done with Lua in C++.\n");

    lua_close(L);

    return 0;
}
