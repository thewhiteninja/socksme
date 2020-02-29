#pragma once

#include <vector>
#include <string>

#include <ws2tcpip.h>
#include <Windows.h>

typedef enum _loglevel {
    NONE = -1,
	ERR = 0,
	INFO = 1,
	DEBUG = 2
} loglevel;

namespace utils
{
	bool is_valid_port(uint32_t port);

	bool is_valid_ip(std::string ip);

    std::string basename(const std::string& str);

    std::string ip_to_str(const struct sockaddr *sa);

    uint16_t get_port(const struct sockaddr *sa);

    std::vector<std::uint8_t> intersection(std::vector<std::uint8_t> &v1, std::vector<std::uint8_t> &v2);

}

namespace logs 
{
    void info(std::string message);

	void err(std::string message, int err = 0);

	void debug(std::string message, int err = 0);
}