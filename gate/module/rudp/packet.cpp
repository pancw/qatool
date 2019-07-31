
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "packet.h"


namespace rudp {

#define RUDP_CONNECT_PACKET "rudp_connect_packet"
#define RUDP_BACK_CONV_PACKET "rudp_back_conv_packet"

std::string making_connect_packet()
{
	return std::string(RUDP_CONNECT_PACKET, sizeof(RUDP_CONNECT_PACKET));	
}

bool is_connect_packet(const char* data, size_t len)
{
	return (len == sizeof(RUDP_CONNECT_PACKET) &&
		memcmp(data, RUDP_CONNECT_PACKET, sizeof(RUDP_CONNECT_PACKET) - 1) == 0);	
}

bool is_send_back_conv_packet(const char* data, size_t len)
{
    return (len > sizeof(RUDP_BACK_CONV_PACKET) &&
        memcmp(data, RUDP_BACK_CONV_PACKET, sizeof(RUDP_BACK_CONV_PACKET) - 1) == 0); 
}

std::string making_send_back_conv_packet(unsigned int conv)
{
    char str_send_back_conv[256] = ""; 
    size_t n = snprintf(str_send_back_conv, sizeof(str_send_back_conv), "%s %u", RUDP_BACK_CONV_PACKET, conv);
    return std::string(str_send_back_conv, n); 
}

unsigned int grab_conv_from_send_back_conv_packet(const char* data, size_t len)
{
    unsigned int conv = atol(data + sizeof(RUDP_BACK_CONV_PACKET));
    return conv;
}

}
