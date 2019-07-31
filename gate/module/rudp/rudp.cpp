
#include "rudp.h"
#include "packet.h"
#include "connection_mgr.h"

namespace rudp {

char message[BUF_SIZE+1] = {};
int serv_sock = -1;
static bool set_nonblock(int sockfd)
{
	int flag = fcntl(sockfd, F_GETFL, 0);
	if (flag < 0)
	{
		fprintf(stderr, "fcntl F_GETFL error!\n");
		return false;
	}
	if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0)
	{
		fprintf(stderr, "fcntl F_SETFL error!\n");
		return false;
	}
	return true;
}

int get_serv_sock()
{
	return serv_sock;
}

bool listen_udp_client(int port)
{
	struct sockaddr_in serv_adr;
	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (serv_sock == -1 )
	{
		printf("udp socket createtion error!\n");
		return false;
	}
	if (!set_nonblock(serv_sock))
	{
		return false;
	}

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);	
	serv_adr.sin_port = htons(port); // atoi

	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
	{
		
		printf("udp socket bind error!\n");
		return false;
	}
	printf("udp listening on port %d.\n", port);	
	return true;
}

void poll_event()
{
	struct sockaddr_in clnt_adr;
	bzero(&clnt_adr, sizeof(clnt_adr));
	socklen_t clnt_adr_sz = sizeof(clnt_adr);	
	int str_len = recvfrom(serv_sock, message, BUF_SIZE, 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
	if (str_len >= 0)
	{
		//std::string s (message, str_len);
		//std::cout << "sock recv:" << s << std::endl;
		//ikcp_input(kcp1, message, str_len);
		//printf("udp event cnt:%d\n", str_len);
		//sendto(serv_sock, message, str_len, 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);
		if (is_connect_packet(message, str_len))
		{
			handle_connect_packet(message, str_len, clnt_adr);
		}
		else
		{
			handle_kcp_packet(message, str_len);				
		}
	}
	update_all_connection();
}

void release()
{
	release_all_connections();
	if (serv_sock != -1)
	{
		close(serv_sock);
	}
	printf("rudp released.\n");	
}

}
