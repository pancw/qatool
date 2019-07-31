

#ifndef __UNIX_SOCKET_SERVER_H__
#define __UNIX_SOCKET_SERVER_H__

#include <signal.h>
#include <sys/socket.h>

#include <sys/types.h>

#include <event2/event-config.h>

#include <sys/stat.h>
#ifndef _WIN32
#include <sys/queue.h>
#include <unistd.h>
#endif
#include <time.h>
#ifdef EVENT__HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <unordered_map>
#include <arpa/inet.h>
#include <iostream>
#include "connectionBase.h"

namespace unixSocketServer {

union ipc_user_data {
	int ufd;
	void *p;
};

class acceptIpcClient: public commonConnection::ConnectionBase
{
public:	
	acceptIpcClient(int,int);
	int ufd;
	int socket_fd;
	~acceptIpcClient();

	int get_ufd(){return this->ufd;};

	void handle_event(const char*, size_t);
	void read_error();

private:
};

bool listen_unix_client(int srv_id);
void release();
void open_libs(lua_State* L);
}

#endif
