
#ifndef __LIVEVENT_BASE_H__
#define __LIVEVENT_BASE_H__

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

#include <map>
#include <arpa/inet.h>
#include <iostream>
#include "connectionBase.h"

namespace libeventBase {

union cb_user_data {
	unsigned int vfd;
	void *p;
};

class Client :public commonConnection::ConnectionBase
{
public:
	Client(int fd, unsigned int vfd, struct event_base *evBase, std::string);
	~Client();

	int get_fd(){
		return fd;
	}

	void handle_event(const char*, size_t);
	void read_error();

	unsigned int getVfd()
	{
		return this->vfd;
	}

	const char* getIp()
	{
		return (this->ip).c_str();
	}

private:
	unsigned int vfd;
	int fd;
	std::string ip;
};


bool init_base();
bool listen_tcp_client(int);
event_base* get_base();
void poll_event();
bool is_running();
void try_connect_unix_srv(int);
void release();
void dispatch();
void open_libs(lua_State* L);
}

#endif
