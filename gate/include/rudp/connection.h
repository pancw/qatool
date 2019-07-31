

#ifndef __RUDP_CONNECTION_H__
#define __RUDP_CONNECTION_H__

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "rudp.h"
#include "ikcp.h"

namespace rudp {

class Connection 
{
public:
	Connection();
	~Connection();
	void init_conn(unsigned int, struct sockaddr_in);
	void send_packet(std::string package);
	void send_packet(const char* msg, size_t len);
	int send_udp_packet(const char *buf, int len);
	void input_packet(const char*msg, size_t len);
	char buf[BUF_SIZE];
	bool kcp_recv();
	ikcpcb* get_kcp(){ return kcp;};

private:
	struct sockaddr_in& get_addr(){ return addr; };
	unsigned int conv;
	ikcpcb* kcp;
	struct sockaddr_in addr;
};


}

#endif
