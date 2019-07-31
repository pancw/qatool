#include <iostream>
#include <cstdio>
#include "luaBase.h"
#include "luaService.h"
#include "libeventBase.h"
#include "unix_socket_server.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
extern int luaopen_cmsgpack(lua_State *);
}

static void luaopen_libs(lua_State * L)
{
	luaopen_cmsgpack(L);
	unixSocketServer::open_libs(L);
}

static void releaseAll()
{
	unixSocketServer::release();
	libeventBase::release();
	printf("All released.\n");
}

int main(int argc, char * argv[])
{

	if (argc < 2)
	{
		printf("Usage:%s <server id>\n", argv[0]);
		return -1;
	}

	if (chdir("logic") == -1)
	{
		fprintf(stderr, "bad logic path to dir:%s\n", "../logic");
		return 1;
	}

	// init
	luaBase::initLua();
	if (!libeventBase::init_base())
	{
		fprintf(stderr, "libevent base init error!\n");
		return 1;
	}
	int srv_id = atoi(argv[1]);
	if (!unixSocketServer::listen_unix_client(srv_id))
	{
		return 1;
	}

	// libs
	lua_State* L = luaBase::getLuaState();
	luaopen_libs(L);

	// main lua
	lua_pushcclosure(L, luaService::error_fun, 0); 
	int err = luaL_loadfile(L, "main.lua");   
	if (err)
	{   
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return 1;
	}   

	int ret = lua_pcall(L, 0, 0, -2);
	if (ret)
	{   
		fprintf(stderr, "call main error:%s\n", lua_tostring(L, -1));
		return 1;
	}
	luaService::call(luaBase::getLuaState(), "BeforLoop", srv_id);

	/*
	while (libeventBase::is_running())
	{
		libeventBase::poll_event();
		usleep(10000);
	}
	*/

	libeventBase::dispatch();
	releaseAll();
	return 0;
}
