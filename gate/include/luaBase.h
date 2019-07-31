
#ifndef __LUA_BASE_H__
#define __LUA_BASE_H__

#include <iostream>
#include <cstdio>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace luaBase{

lua_State* getLuaState();
void initLua();
int error_fun(lua_State *state);

}

#endif
