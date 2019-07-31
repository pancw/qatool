
#ifndef __LMONGO_H_
#define __LMONGO_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace lmongoc {
bool connect_mongo(const char*);
void release();
bool insert();
void find();
void deleting();
void update();
void executing();
void createDoc();
void testDoc();
void luaopen_mongoc(lua_State*);
}
#endif
