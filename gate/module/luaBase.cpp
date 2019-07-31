#include "luaBase.h"
namespace luaBase{

static lua_State *GlobalL;
lua_State* getLuaState()
{
	return GlobalL;
}

void initLua()
{
	GlobalL = luaL_newstate();
	luaL_openlibs(GlobalL);
}

int error_fun(lua_State *state)
{
	std::string result;
	const char *tmp = lua_tostring(state, -1); // error_msg
	if (tmp) {
	result = tmp;
	}   

	lua_getglobal(state, "debug"); // error_msg, debug
	lua_getfield(state, -1, "traceback"); // error_msg, debug, traceback
	lua_call(state, 0, 1); // error_msg, traceback

	tmp = lua_tostring(state, -1);
	if (tmp) {
	result = result + "\n" + tmp;
	}   

	lua_pushstring(state, result.c_str()); // push result
	return 1;
}







}
