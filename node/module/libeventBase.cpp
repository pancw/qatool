
#include "libeventBase.h"
#include <sys/un.h>
#include "luaService.h"
#include "luaBase.h"

namespace libeventBase {

static struct event_base* base;
static struct event *timeout;
static bool running = true;
static struct event *signal_int;
struct evconnlistener *listener = NULL;

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

void release()
{
	event_base_free(base);
	printf("free libevent base.\n");
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

}
