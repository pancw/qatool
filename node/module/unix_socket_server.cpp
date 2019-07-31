
#include "unix_socket_server.h"
#include <sys/un.h>
#include "libeventBase.h"
#include <iostream>
#include <vector>
#include "luaService.h"
#include "luaBase.h"

namespace unixSocketServer {

#define UNIX_DOMAIN "../../unix.domain"
int unix_listen_fd = -1;
struct event *unix_listener = NULL;
unsigned int globalUfd = 1;
std::unordered_map<int, acceptIpcClient*> allAcceptIpcClient;

static acceptIpcClient* query_ipc_client(int ufd)
{
	std::unordered_map<int, acceptIpcClient*>::iterator it = allAcceptIpcClient.find(ufd);
	if (it == allAcceptIpcClient.end())
	{
		return NULL;
	}
	return it->second;
}

static void release_ipc_client(int ufd)
{
	acceptIpcClient* c = query_ipc_client(ufd);
	if (c)
	{
		allAcceptIpcClient.erase(ufd);
		delete c;
		printf("release accept ipc client:%d\n", ufd);
	}
	else
	{
		fprintf(stderr, "release_ipc_client error!ufd:%d\n", ufd);
	}
}

void acceptIpcClient::handle_event(const char* msg, size_t len)
{
	lua_State* GlobalL = luaBase::getLuaState();
	int top = lua_gettop(GlobalL);
	lua_pushcclosure(GlobalL, luaBase::error_fun, 0);
	lua_getglobal(GlobalL, "RecvJob");
	lua_pushnumber(GlobalL, this->get_ufd());
	lua_pushlstring(GlobalL, msg, len);
	
	int result = lua_pcall(GlobalL, 2, 0, -2-2);
	if (result)
	{
		printf("[lua-call(%d)]: %s\n", 1, lua_tostring(GlobalL, -1));
	}
	lua_settop(GlobalL, top);
}

void acceptIpcClient::read_error()
{
	release_ipc_client(this->get_ufd());
}

void unix_srv_msg_cb(struct bufferevent* bev, void* arg)
{
	ipc_user_data usd;
	usd.p = arg;
	int ufd = usd.ufd;

	acceptIpcClient* c = query_ipc_client(ufd);
	if (c)
	{
		c->do_read(bev);
	}
	else
	{
		fprintf(stderr, "read data no client!\n");
	}
}

void unix_srv_event_cb(struct bufferevent *bev, short event, void *arg)
{
	ipc_user_data usd;
	usd.p = arg;
	int ufd = usd.ufd;
	release_ipc_client(ufd);
}

acceptIpcClient::~acceptIpcClient()
{
	if (this->socket_fd != -1)
	{
		close(this->socket_fd);
	}
}

acceptIpcClient::acceptIpcClient(int ufd, int sock_fd):ConnectionBase()
{
	this->ufd = ufd;	
	this->socket_fd = sock_fd;

	int flags = fcntl(this->socket_fd, F_GETFL, 0); 
	flags |= O_NONBLOCK;
	fcntl(this->socket_fd, F_SETFL, flags);

	this->bev = bufferevent_socket_new(libeventBase::get_base(), this->socket_fd, BEV_OPT_CLOSE_ON_FREE);
	ipc_user_data usd;
	usd.ufd = this->ufd;
	bufferevent_setcb(this->bev, unix_srv_msg_cb, NULL, unix_srv_event_cb, usd.p);
	bufferevent_enable(this->bev, EV_READ | EV_PERSIST);  	
}

void release()
{
	if (unix_listen_fd != -1)
	{
                close(unix_listen_fd);
                unix_listen_fd = -1; 
	}
	if (unix_listener)
	{
		event_free(unix_listener);
	}

	std::vector<int> v;
	std::unordered_map<int, acceptIpcClient*>::iterator it = allAcceptIpcClient.begin();
	for (; it != allAcceptIpcClient.end(); it++)
	{
		v.push_back(it->first);	
	}

	std::vector<int>::iterator vit = v.begin();
	for (; vit != v.end(); vit++)
	{
		release_ipc_client(*vit);		
	}
}

void new_unix_client(int fd, short event, void *arg)
{   
	struct sockaddr_in addr;
	int len = sizeof(addr);
	bzero(&addr, len);

	int unix_fd = accept(fd, (struct sockaddr*)&addr, (socklen_t*)&len);
	if ( unix_fd < 0 ) 
	{   
		printf("Accept Error:\n");
		return;
	}   
	int ufd = globalUfd++;
	acceptIpcClient* c = new acceptIpcClient(ufd, unix_fd);	
	allAcceptIpcClient[ufd] = c;
	luaService::call(luaBase::getLuaState(), "GateConnected", ufd);
}


bool listen_unix_client(int srv_id)
{
	std::string file_name = UNIX_DOMAIN + std::to_string(srv_id);
        unix_listen_fd = socket(AF_LOCAL, SOCK_STREAM, 0); 
        int iFlags = fcntl(unix_listen_fd, F_GETFL, 0); 
        if (iFlags == -1 || fcntl(unix_listen_fd, F_SETFL, iFlags | O_NONBLOCK))
        {   
                close(unix_listen_fd);
                unix_listen_fd = -1; 
		fprintf(stderr, "unix socket error!file_name:%s\n", file_name.c_str());
                return false;
        }   
        struct sockaddr_un addr;
        bzero(&addr, sizeof(addr));
        addr.sun_family = AF_LOCAL;

	strncpy(addr.sun_path,file_name.c_str(),sizeof(addr.sun_path)-1);
	unlink(file_name.c_str());

        bind(unix_listen_fd, (struct sockaddr * ) &addr, sizeof(addr));

        if (listen(unix_listen_fd, SOMAXCONN)==-1)
	{
                close(unix_listen_fd);
                unix_listen_fd = -1; 
		fprintf(stderr, "unix listen error!file_name:%s\n", file_name.c_str());
		return false;
	}

        unix_listener = event_new(libeventBase::get_base(), unix_listen_fd, EV_READ|EV_PERSIST, new_unix_client, NULL);
        event_add(unix_listener, NULL);
        printf("engine bind on unix socket:%s\n", file_name.c_str());

	return true;	
}

static int ack_job_to_unix_client(lua_State* L)
{
	int ufd = lua_tonumber(L, 1);	
	size_t size;
	const char* msg = lua_tolstring(L, 2, &size);
	if (!msg)
	{
		fprintf(stderr, "ack null job!\n");
		return 0;
	}

	acceptIpcClient* c = query_ipc_client(ufd);
	if (!c)
	{
		fprintf(stderr, "ack job to null ipc client ! ufd:%d\n", ufd);
		return 0;
	}

	if (size > commonConnection::max_body_length)
	{
		fprintf(stderr, "ack data too long!\n");
		return 0;
	}
	else
	{
		c->do_write(msg, size);
	}
	lua_pushboolean(L, true);
	return 1;
}

const luaL_reg libs[] =
{
	{"ack", ack_job_to_unix_client},
	{NULL, NULL},
};

void open_libs(lua_State* L)
{
	luaL_register(L, "IPC", libs);	
}

}
