
#include "libeventBase.h"
#include "luaService.h"
#include "luaBase.h"
#include "unix_socket_client.h"
#include <unordered_map>
#include <vector>

namespace libeventBase {

static struct event_base* base;
static struct event *timeout;
static bool running = true;
static struct event *signal_int;
struct evconnlistener *listener = NULL;

unsigned int globalVfd = 0;
std::unordered_map<unsigned int, Client*> allClients;

static Client* queryClient(unsigned int vfd)
{
	std::unordered_map<unsigned int, Client*>::iterator it = allClients.find(vfd);
	if (it == allClients.end())
	{
		return NULL;
	}
	return it->second;
}

static void tryReleaseClient(unsigned int vfd)
{
	Client* c = queryClient(vfd);
	if (c)
	{
		allClients.erase(vfd);
		delete c;
		printf("release a tcp client.%d\n", vfd);
	}	
	else
	{
		fprintf(stderr, "tryReleaseClient error!vfd:%d\n", vfd);
	}
	// lua_disconnect(vfd);
	luaService::call(luaBase::getLuaState(), "Disconnect", vfd);
}

void release()
{
	std::vector<int> v;
	std::unordered_map<unsigned int, Client*>::iterator it = allClients.begin();
	for (; it != allClients.end(); it++)
	{
		v.push_back(it->first);	
	}

	std::vector<int>::iterator vit = v.begin();
	for (; vit != v.end(); vit++)
	{
		tryReleaseClient(*vit);		
	}
	event_base_free(base);
	printf("free libevent base.\n");
}

bool is_running()
{
	return running;
}

event_base* get_base()
{
	return base;
}

static void tick(evutil_socket_t fd, short event, void *arg)
{   
	luaService::call(luaBase::getLuaState(), "Tick");
	fflush ( stdout ) ;
}   

static void signal_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *signal = (struct event*)arg;
	printf("signal_cb: got signal %d\n", event_get_signal(signal));

	running = false;
	event_del(signal);
	event_base_loopbreak(base);
	event_free(signal_int);
	event_free(timeout);
}

static void
tcp_listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *user_data)
{
	unsigned vfd = ++globalVfd;
	struct event_base *evBase = (struct event_base *)user_data;
	std::string ip = inet_ntoa(((sockaddr_in*)sa)->sin_addr);
	Client* client = new Client(fd, vfd, evBase, ip);
	if (!allClients.insert(std::make_pair(vfd, client)).second)
	{
		close(fd);
		delete client;
		fprintf(stderr, "client insert error!\n");
		return;
	}
	
	printf("accept a tcp client. \n");
}

bool listen_tcp_client(int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	listener = evconnlistener_new_bind(base, tcp_listener_cb, (void *)base,
	    LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
	    (struct sockaddr*)&sin,
	    sizeof(sin));

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return false;
	}
	printf("Accepting tcp client connections on port %d.\n", port);
	return true;	
}

bool init_base()
{
	struct timeval tv;
	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return false;
	}

	signal_int = evsignal_new(base, SIGINT, signal_cb, event_self_cbarg());
	event_add(signal_int, NULL);

	timeout = event_new(base, -1, EV_PERSIST, tick, (void*) timeout);
	evutil_timerclear(&tv);
	tv.tv_sec = 1;
	event_add(timeout, &tv);
	return true;
}

void poll_event()
{
	event_base_loop(base, EVLOOP_ONCE | EVLOOP_NONBLOCK);
}

void dispatch()
{
	event_base_dispatch(base);          
}

static void socket_event_cb(struct bufferevent *bev, short events, void *arg)
{
	cb_user_data usd;
	usd.p = arg;
	unsigned int vfd = usd.vfd;

	if (events & BEV_EVENT_EOF)
		printf("vfd:%d connection close.\n", vfd);
	else if (events & BEV_EVENT_ERROR)
		printf("vfd:%d some other error!\n", vfd);

	tryReleaseClient(vfd);
}

static void socket_read_cb(struct bufferevent *bev, void *arg)
{
	cb_user_data usd;
	usd.p = arg;
	unsigned int vfd = usd.vfd;

	Client* client = queryClient(vfd);	
	if (client)
	{
		client->do_read(bev);
	}
	else
	{
		fprintf(stderr, "read data no client!\n");
	}
}
// ---------------------------------- tcp client -----------------------------------
Client::Client(int fd, unsigned int vfd, struct event_base *evBase, std::string clientIp):ConnectionBase()
{
	this->vfd = vfd;
	this->fd = fd;
	this->bev = bufferevent_socket_new(evBase, fd, BEV_OPT_CLOSE_ON_FREE);
	this->ip = clientIp;

	cb_user_data usd;
	usd.vfd = vfd;
	bufferevent_setcb(this->bev,  socket_read_cb, NULL, socket_event_cb, usd.p);
	bufferevent_enable(this->bev, EV_READ|EV_WRITE);//|EV_PERSIST);
}

Client::~Client()
{
	if (this->fd >0)
		close(this->fd);
}


void Client::handle_event(const char* msg, size_t len)
{
	lua_State* GlobalL = luaBase::getLuaState();
	int top = lua_gettop(GlobalL);
	lua_pushcclosure(GlobalL, luaBase::error_fun, 0);
	lua_getglobal(GlobalL, "RecvJob");
	lua_pushnumber(GlobalL, this->getVfd());
	lua_pushlstring(GlobalL, msg, len);
	
	int result = lua_pcall(GlobalL, 2, 0, -2-2);
	if (result)
	{
		printf("[lua-call(%d)]: %s\n", 1, lua_tostring(GlobalL, -1));
	}
	lua_settop(GlobalL, top);
}

void Client::read_error()
{
	tryReleaseClient(this->getVfd());
}

static int send_msg_to_tcp_client(lua_State* L)
{
	int vfd = lua_tonumber(L, 1);	
	size_t size;
	const char* msg = lua_tolstring(L, 2, &size);
	if (!msg)
	{
		fprintf(stderr, "send null msg!\n");
		return 0;
	}

	Client* c = queryClient(vfd);
	if (!c)
	{
		fprintf(stderr, "send msg to null tcp client !\n");
		return 0;
	}

	if (size > commonConnection::max_body_length)
	{
		const char * temp = "msg too long";
		fprintf(stderr, "send data too long!\n");
		c->do_write(temp, strlen(temp));
	}
	else
	{
		c->do_write(msg, size);
	}
	lua_pushboolean(L, true);
	return 1;
}

static int kick_tcp_client(lua_State* L)
{
	int vfd = lua_tonumber(L, 1);	
	tryReleaseClient(vfd);		
	return 0;
}

const luaL_reg libs[] =
{
	{"send", send_msg_to_tcp_client},
	{"kick", kick_tcp_client},
	{NULL, NULL},
};

void open_libs(lua_State* L)
{
	luaL_register(L, "NET", libs);	
}

}
