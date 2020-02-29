#pragma once

#include "options.h"

#include <ws2tcpip.h>
#include <Windows.h>

#include <thread>


class SocksServer
{
private:
    std::string	    _local_ip;
    uint32_t		_local_port;

    SOCKET		    _socket;

	bool			_is_ready;

public:
    explicit SocksServer();
    ~SocksServer();

	void run();

	bool is_ready() const { return _is_ready; }

};