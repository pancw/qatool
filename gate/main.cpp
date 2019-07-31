#include <iostream>
#include <cstdio>
#include "luaBase.h"
#include "luaService.h"
#include "libeventBase.h"
#include "unix_socket_client.h"
#include "lmongo.h"
#include "rudp.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
extern int luaopen_cmsgpack(lua_State *);
}

static void luaopen_libs(lua_State * L)
{
	lmongoc::luaopen_mongoc(L);
	luaopen_cmsgpack(L);
	unixSocketClient::open_libs(L);
	libeventBase::open_libs(L);
}

static void releaseAll()
{
	unixSocketClient::release();
	rudp::release();
	libeventBase::release();
	lmongoc::release();
	printf("All released.\n");
}

int main(int argc, char * argv[])
{
	if (argc < 3)
	{
		printf("Usage:%s <node cnt> <port>\n", argv[0]);
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

	int node_cnt = atoi(argv[1]);
	if (!unixSocketClient::init(node_cnt))
	{
		fprintf(stderr, "node cnt must > 0\n");
		return 1;
	}

	// config
	lua_State* GlobalL = luaBase::getLuaState();
	lua_getglobal(GlobalL, "GetGlobalConfig");
	ret = lua_pcall(GlobalL, 0, 1, 0);
	if (ret)
	{   
		fprintf(stderr, "call GetGlobalConfig error:%s\n", lua_tostring(GlobalL, -1));
		return 1;
	}
	const char* db_url = lua_tostring(GlobalL, -1);
	if (!db_url)
	{
		fprintf(stderr, "db url error!\n");
		return 1;
	}
	if (!lmongoc::connect_mongo(db_url))
	{
		return 1;
	}

	if (!libeventBase::listen_tcp_client(atoi(argv[2])))
	{
		return 1;
	}

	if (!rudp::listen_udp_client(atoi(argv[2])+1))
	{
		return 1;
	}

	luaService::call(luaBase::getLuaState(), "BeforLoop");

	while (libeventBase::is_running())
	{
		libeventBase::poll_event();
		rudp::poll_event();
		usleep(10000);
	}
	// libeventBase::dispatch();
	luaService::call(luaBase::getLuaState(), "BeforShutdown");
	releaseAll();
	return 0;
}
