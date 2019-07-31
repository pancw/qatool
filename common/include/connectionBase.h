
#ifndef __COMMON_ACCEPT_CLIENT_H__
#define __COMMON_ACCEPT_CLIENT_H__

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

namespace commonConnection {

const unsigned char StatusReadHeader = 1;
const unsigned char StatusReadbody = 2;

enum { header_length = 2 };
enum { max_body_length = 60000 }; // max:256*256+256

class ConnectionBase 
{
public:
	ConnectionBase();
	virtual ~ConnectionBase();//  warning: deleting object of polymorphic class type ‘unixSocketClient::ipcClient’ which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor] 虚函数才能使父类指针指向子类，析构时子类析构函数被调用

	bool do_write(const char* line, size_t size){
		memcpy(this->write_msg + header_length, line, size);                                  
		// encode header
		this->write_msg[0] = (unsigned char)(size % 256);
		this->write_msg[1] = (unsigned char)(size / 256);

		bufferevent_write(this->bev, this->write_msg, header_length + size);    
		return true;
	}

	void do_read(struct bufferevent* bufev);
	virtual void handle_event(const char*, size_t)
	{
		
		fprintf(stderr, "base handle event error!\n");
	}
	virtual void read_error()
	{
		fprintf(stderr, "base read error error!\n");
	}

	struct bufferevent * get_bev(){
		return this->bev;
	}

protected:
	struct bufferevent *bev;

private:
	unsigned char read_status;
	size_t need_byte_cnt;
	size_t readed_byte_cnt;

	void set_read_status(unsigned char status){
		this->read_status = status;
	}

	unsigned char get_read_status(){
		return this->read_status;
	}

	void reset();

	char write_msg[header_length + max_body_length];
	char read_msg[header_length + max_body_length];
};

}

#endif
