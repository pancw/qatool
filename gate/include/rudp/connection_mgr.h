
#ifndef __RUDP_CONNECTION_MGR_H__
#define __RUDP_CONNECTION_MGR_H__

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//#include <net/if.h>

namespace rudp {

void handle_connect_packet(const char* msg, size_t size, struct sockaddr_in& addr);
void handle_kcp_packet(const char* msg, size_t size);
void update_all_connection();
void release_all_connections();

}

#endif
