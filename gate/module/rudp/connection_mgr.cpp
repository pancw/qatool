
#include "connection_mgr.h"
#include "connection.h"
#include <unordered_map>
#include "packet.h"
#include "rudp.h"
#include "util.h"
#include <vector>


namespace rudp {

unsigned int global_conv = 1111;
std::unordered_map<unsigned int, Connection*> allConnections;

static Connection* query_conn(unsigned int conv)
{
	std::unordered_map<unsigned int, Connection*>::iterator it = allConnections.find(conv);
	if (it == allConnections.end())
	{
		return NULL;
	}
	return it->second;
}

static void try_release_connection(unsigned int conv)
{
	Connection* c = query_conn(conv);	
	if (c)
	{
		allConnections.erase(conv);
		delete c;
		printf("release a rudp client.%d\n", conv);
	}
}

void release_all_connections()
{
	std::vector<int> v;
	std::unordered_map<unsigned int, Connection*>::iterator it = allConnections.begin();
	for (; it != allConnections.end(); it++)
	{
		v.push_back(it->first);	
	}

	std::vector<int>::iterator vit = v.begin();
	for (; vit != v.end(); vit++)
	{
		try_release_connection(*vit);		
	}
}

void update_all_connection()
{
	std::unordered_map<unsigned int, Connection*>::iterator it = allConnections.begin();
	for (; it != allConnections.end(); it++)
	{
		Connection* c = it->second;
		ikcp_update(c->get_kcp(), iclock());
		c->kcp_recv();
	}
}

void handle_connect_packet(const char* msg, size_t size, struct sockaddr_in &addr)
{
	unsigned int conv = global_conv++;	
	Connection* c = new Connection();	
	allConnections[conv] = c;
	c->init_conn(conv, addr);
	std::string send_back_packet = making_send_back_conv_packet(conv);
	//c->send_udp_packet(send_back_packet.c_str(),send_back_packet.length());
	c->send_packet(send_back_packet);

	printf("accept a rudp client. conv:%u\n",conv);
	//std::string p(msg, size);
	//std::cout << "accept rudp:" << p << std::endl << send_back_packet << std::endl;
}

void handle_kcp_packet(const char* msg, size_t size)
{
	unsigned int conv = ikcp_getconv(msg);
	if (conv == 0)
	{   
		fprintf(stderr, "ikcp_get_conv return 0\n");
		return;
	}
	Connection* c = query_conn(conv);
	if (!c)
	{
		
		fprintf(stderr, "handle conv no conn!\n");
		return;
	}
	c->input_packet(msg, size);
	c->kcp_recv();
}

}
