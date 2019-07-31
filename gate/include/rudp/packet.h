
#ifndef __PACKET_H__
#define __PACKET_H__

#include <iostream>

namespace rudp {

std::string making_connect_packet();
bool is_connect_packet(const char* data, size_t len);
bool is_send_back_conv_packet(const char* data, size_t len);
std::string making_send_back_conv_packet(unsigned int conv);
unsigned int grab_conv_from_send_back_conv_packet(const char* data, size_t len);


}

#endif
