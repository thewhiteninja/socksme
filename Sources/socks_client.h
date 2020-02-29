#pragma once


#include <ws2tcpip.h>
#include <Windows.h>

#include <cstdint>
#include <string>


#define AUTH_METHOD_NO_AUTH_REQUIRED        0x00
#define AUTH_METHOD_GSSAPI                  0x01
#define AUTH_METHOD_USER_PASSWORD           0x02
#define AUTH_METHOD_NO_ACCEPTABLE_METHODS   0xff

#define SOCKS_COMMAND_CONNECT               0x01
#define SOCKS_COMMAND_BIND                  0x02
#define SOCKS_COMMAND_UPD_ASSOCIATE         0x03

#define SOCKS_REQUEST_TYPE_IPV4             0x01
#define SOCKS_REQUEST_TYPE_DOMAIN           0x03
#define SOCKS_REQUEST_TYPE_IPV6             0x04

#define SOCKET_RESPONSE_SUCCEEDED               0x00
#define SOCKET_RESPONSE_SERVER_FAILURE          0x01
#define SOCKET_RESPONSE_NOT_ALLOWED             0x02
#define SOCKET_RESPONSE_NETWORK_UNREACHABLE     0x03
#define SOCKET_RESPONSE_HOST_UNREACHABLE        0x04
#define SOCKET_RESPONSE_REFUSED                 0x05
#define SOCKET_RESPONSE_TTL_EXPIRED             0x06
#define SOCKET_RESPONSE_COMMAND_NOT_SUPPORTED   0x07
#define SOCKET_RESPONSE_ADDRESS_NOT_SUPPORTED   0x08

#pragma pack(push, 1)

typedef struct _socks_request
{
    uint8_t version;
    uint8_t command;
    uint8_t reserved;
    uint8_t type;
} socks_request;

#pragma pack(pop) 

class SocksClient{

private:
	SOCKET		_socket;
	std::string _ip;
	uint16_t		_port;

    uint32_t _id;

    static uint32_t _count;

    DWORD _authenticate();
    DWORD _exec_command();
    DWORD _send_response(uint8_t resp, SOCKET sock);
    DWORD _connect_socket_v4(SOCKET* s, std::string addr, uint16_t port);
    DWORD _connect_socket_v6(SOCKET* s, std::string addr, uint16_t port);
    DWORD _connect_socket_domain(SOCKET * s, std::string domain, uint16_t port);
    bool  _validate_user(std::string username, std::string password);

public:
	explicit SocksClient(SOCKET socket);
	~SocksClient();

	uint16_t port() const { return _port; }
	std::string ip() const { return _ip; }

	void handle();
};
