#ifndef __LUA_SERVICE_H_
#define __LUA_SERVICE_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <iostream>
#include <cstdio>

namespace luaService{
	inline void push_value(lua_State*L, bool value)
	{
		lua_pushboolean(L, value);
	}	

	inline void push_value(lua_State*L, int value)
	{
		lua_pushnumber(L, value);
	}

	inline void push_value(lua_State*L, unsigned int value)
	{
		lua_pushnumber(L, value);
	}

	inline void push_value(lua_State*L, const char* value)
	{
		lua_pushstring(L, value);
	}

	inline void push_value(lua_State*L, double value)
	{
		lua_pushboolean(L, value);
	}	

	int error_fun(lua_State*);
	inline void push_funs( lua_State*L, const std::string &name)
	{
		lua_pushcfunction(L, error_fun); 
		lua_getglobal(L, name.c_str());
	}
	void do_call( lua_State*L, int argc, const std::string &name);

	inline void call( lua_State* L, const std::string &name)
	{
		push_funs(L, name);
		do_call(L, 0, name);
	}

	template < typename T1 >
	void call( lua_State* L, const std::string &name, const T1 &v1)
	{
		push_funs(L, name);
		push_value(L, v1);
		do_call(L, 1, name);
	}

	template < typename T1, typename T2 >
	void call( lua_State* L, const std::string &name, const T1 &v1, const T2 &v2)
	{
		push_funs(L, name);
		push_value(L, v1);
		push_value(L, v2);
		do_call(L, 2, name);
	}

	
	template < typename T1, typename T2, typename T3 >
	void call( lua_State* L, const std::string &name, const T1 &v1, const T2 &v2, const T3 &v3)
	{
		push_funs(L, name);
		push_value(L, v1);
		push_value(L, v2);
		push_value(L, v3);
		do_call(L, 3, name);
	}

	template < typename T1, typename T2, typename T3, typename T4 >
	void call( lua_State* L, const std::string &name, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4)
	{
		push_funs(L, name);
		push_value(L, v1);
		push_value(L, v2);
		push_value(L, v3);
		push_value(L, v4);
		do_call(L, 4, name);
	}
}	
#endif
