#include "luaService.h"

namespace luaService{

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

void do_call(lua_State* L, int argc, const std::string &name)
{
	int result = lua_pcall( L, argc, 0, -argc - 2 );
	if ( result ) {
		printf( "[lua-call-%s(%d)]: %s\n",  name.c_str(), argc, lua_tostring( L, -1 ) );
		lua_pop( L, 1 ); // pop error_msg
	}
	lua_pop( L, 1 ); // pop the error_fun	
}














}
