#include "connectionBase.h"

namespace commonConnection {

void ConnectionBase::reset()
{
	this->need_byte_cnt = header_length;
	this->readed_byte_cnt = 0;
	set_read_status(StatusReadHeader);
}

ConnectionBase::ConnectionBase()
{
	reset();
}

ConnectionBase::~ConnectionBase()
{
	if (this->bev)
		bufferevent_free(this->bev);
}

void ConnectionBase::do_read(struct bufferevent* bufev)
{
	size_t dataLen = evbuffer_get_length(bufferevent_get_input(bufev)); 
	while (dataLen > 0)
	{
		if (get_read_status() == StatusReadHeader)
		{
			size_t len = bufferevent_read(bufev, read_msg + readed_byte_cnt, need_byte_cnt - readed_byte_cnt);
			readed_byte_cnt += len;

			if (readed_byte_cnt == need_byte_cnt)
			{
				// decode header
				need_byte_cnt = (unsigned char)read_msg[0] + ((unsigned char)(read_msg[1]))*256;
				set_read_status(StatusReadbody);
				readed_byte_cnt = 0;
			}
			dataLen -= len;
		}

		if (get_read_status() == StatusReadbody)
		{
			if (need_byte_cnt >= max_body_length)
			{
				fprintf(stderr, "read body error!\n");
				read_error();
				return;
			}

			size_t len = bufferevent_read(bufev, read_msg + readed_byte_cnt, need_byte_cnt - readed_byte_cnt);
			readed_byte_cnt += len;

			if (readed_byte_cnt == need_byte_cnt)
			{
				handle_event(read_msg, need_byte_cnt);
				reset();
			}
			dataLen -= len;
		}	
	}
}


}
