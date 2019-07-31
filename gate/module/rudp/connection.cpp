
#include "connection.h"

namespace rudp {

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	return ((Connection*)user)->send_udp_packet(buf, len);
}

int Connection::send_udp_packet(const char *buf, int len)
{
	struct sockaddr_in addr = this->get_addr();
	socklen_t addr_sz = sizeof(addr);
	int serv_sock = get_serv_sock();
	int ret = sendto(serv_sock, buf, len, 0, (struct sockaddr*)&addr, addr_sz);
	if (ret == -1)
	{
		fprintf(stderr, "send udp -1\n");
	}
	return ret;
}

void Connection::send_packet(std::string package)
{
	ikcp_send(kcp, package.c_str(), package.length());
}

void Connection::send_packet(const char* msg, size_t len)
{
	ikcp_send(kcp, msg, len);
}

Connection::Connection()
{

}

Connection::~Connection()
{
	ikcp_release(kcp);
}

void Connection::input_packet(const char*msg, size_t len)
{
	ikcp_input(kcp, msg, len);
}

bool Connection::kcp_recv()
{
	int str_len = ikcp_recv(kcp, buf, BUF_SIZE);
	if (str_len > 0)
	{
		buf[str_len] = 0;
		printf("%s\n", buf);
		// TODO
		/*
		std::string s (buf, str_len);
		std::cout << s << std::endl;
		*/
		send_packet(buf, str_len);
		return true;	
	}
	return false;
}

void Connection::init_conn(unsigned int conv, struct sockaddr_in addr)
{
	this->conv = conv;	
	kcp = ikcp_create(conv, (void*)this);
	kcp->output = udp_output;
	ikcp_nodelay(kcp, 1, 5, 1, 1);
	kcp->rx_minrto = 10;
	kcp->fastresend = 1;
	this->addr = addr;
}

}
