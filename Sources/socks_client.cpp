#include "socks_client.h"

#include "utils.h"

#include <thread>
#include <sstream>
#include <vector>

uint32_t SocksClient::_count = 0;

std::vector<uint8_t> supported_auth_methods = {
    AUTH_METHOD_NO_AUTH_REQUIRED,
    AUTH_METHOD_USER_PASSWORD
};

DWORD SocksClient::_authenticate()
{
    int res = 0;

    uint8_t version = 0;
    res = recv(_socket, (char*)&version, 1, 0);
    if (res == -1)
    {
        logs::err("No socks version provided", WSAGetLastError());
        return 1;
    }
    if (version != 5)
    {
        logs::err("Invalid socks version " + std::to_string(version));
        return 2;
    }

    logs::debug("Socks version " + std::to_string(version));

    uint8_t auth_method_count = 0;
    res = recv(_socket, (char*)&auth_method_count, 1, 0);
    if (res == -1 || auth_method_count == 0)
    {
        logs::err("No auth methods count provided", WSAGetLastError());
        return 3;
    }

    std::vector<uint8_t> client_auth_methods;
    for (int i = 0; i < auth_method_count; i++)
    {
        uint8_t method = 0;
        res = recv(_socket, (char*)&method, 1, 0);
        if (res == -1)
        {
            logs::err("Missing auth methods from client", WSAGetLastError());
            return 4;
        }
        client_auth_methods.push_back(method);
    }

    std::vector<uint8_t> accepted_method = utils::intersection(client_auth_methods, supported_auth_methods);
    if (accepted_method.size() == 0)
    {
        logs::err("No accepted auth methods found");
        return 5;
    }

    logs::debug("Auth method : " + std::to_string(accepted_method.at(0)));

    char resp[2] = { (char)version, (char)accepted_method.at(0) };
    send(_socket, resp, 2, 0);

    switch (accepted_method.at(0))
    {
    case AUTH_METHOD_NO_AUTH_REQUIRED:
        break;

    case AUTH_METHOD_USER_PASSWORD:
    {
        uint8_t auth_version = 0;
        res = recv(_socket, (char*)&auth_version, 1, 0);
        if (res == -1)
        {
            logs::err("Missing auth version from client", WSAGetLastError());
            return 6;
        }
        if (auth_version != 1)
        {
            logs::err("Invalid auth version " + std::to_string(auth_version));
            return 7;
        }

        uint8_t username_len = 0;
        res = recv(_socket, (char*)&username_len, 1, 0);
        if (res == -1)
        {
            logs::err("Missing auth username length from client", WSAGetLastError());
            return 8;
        }
        char username[256] = { 0 };
        res = recv(_socket, username, sizeof(username), 0);
        if (res == -1)
        {
            logs::err("Missing auth username from client", WSAGetLastError());
            return 9;
        }

        uint8_t password_len = 0;
        res = recv(_socket, (char*)&password_len, 1, 0);
        if (res == -1)
        {
            logs::err("Missing auth password length from client", WSAGetLastError());
            return 10;
        }
        char password[256] = { 0 };
        res = recv(_socket, password, sizeof(password), 0);
        if (res == -1)
        {
            logs::err("Missing auth password from client", WSAGetLastError());
            return 11;
        }

        char resp[2] = { 0x01, 0x00 };
        if (_validate_user(username, password))
        {
            send(_socket, resp, 2, 0);
            return 0;
        }
        else
        {
            resp[1] = 0x01;
            send(_socket, resp, 2, 0);

            shutdown(_socket, SD_BOTH);
            closesocket(_socket);

            return 33;
        }
    }
    default:
        logs::err("Authentication method not implemented (" + std::to_string(accepted_method.at(0)) + ")");
        return 12;
    }

    return 0;
}

bool  SocksClient::_validate_user(std::string username, std::string password)
{
    bool access = (username == "root" && password == "toor");
    logs::debug("Validating " + username + ":" + password + " ... " + std::to_string(access));
    return access;
}

void set_fds(int sock1, int sock2, fd_set *fds)
{
    FD_ZERO(fds);
    FD_SET(sock1, fds);
    FD_SET(sock2, fds);
}

DWORD SocksClient::_exec_command()
{
    int res = 0;
    char buf[1024] = { 0 };

    uint8_t version = 0;
    res = recv(_socket, (char*)&version, sizeof(version), 0);
    if (res == -1)
    {
        logs::err("No version provided", WSAGetLastError());
        return 13;
    }
    if (version != 5)
    {
        logs::err("Invalid socks version " + std::to_string(version));
        return 14;
    }
    
    uint8_t command = 0;
    res = recv(_socket, (char*)&command, sizeof(command), 0);
    if (res == -1)
    {
        logs::err("No command provided", WSAGetLastError());
        return 15;
    }

    uint8_t reserved = 0;
    res = recv(_socket, (char*)&reserved, sizeof(reserved), 0);
    if (res == -1)
    {
        logs::err("No reserved field provided", WSAGetLastError());
        return 16;
    }
    if (reserved != 0)
    {
        logs::err("Invalid reserved value : " + std::to_string(reserved));
        return 17;
    }

    switch (command)
    {
    case SOCKS_COMMAND_CONNECT:
    {
        uint8_t type = 0;
        res = recv(_socket, (char*)&type, sizeof(type), 0);
        if (res == -1)
        {
            logs::err("No command type provided", WSAGetLastError());
            return 18;
        }

        std::string addr;
        uint16_t    port;

        switch (type)
        {
        case SOCKS_REQUEST_TYPE_IPV4:
        {
            char buf_addr[4] = { 0 };
            res = recv(_socket, buf_addr, sizeof(buf_addr), 0);
            if (res == -1)
            {
                logs::err("No address content provided", WSAGetLastError());
                return 19;
            }
            char tmp_ip[32] = { 0 };
            inet_ntop(AF_INET, buf_addr, tmp_ip, INET_ADDRSTRLEN);
            addr = std::string(tmp_ip);

            res = recv(_socket, buf_addr, 2, 0);
            if (res == -1)
            {
                logs::err("No port content provided", WSAGetLastError());
                return 20;
            }
            port = ntohs(*((uint16_t*)buf_addr));
            break;
        }
        case SOCKS_REQUEST_TYPE_IPV6:
        {
            char buf_addr[16] = { 0 };
            res = recv(_socket, buf_addr, sizeof(buf_addr), 0);
            if (res == -1)
            {
                logs::err("No address content provided", WSAGetLastError());
                return 21;
            }
            char tmp_ip[128] = { 0 };
            inet_ntop(AF_INET6, buf_addr, tmp_ip, INET6_ADDRSTRLEN);
            addr = std::string(tmp_ip);

            res = recv(_socket, buf_addr, 2, 0);
            if (res == -1)
            {
                logs::err("No port content provided", WSAGetLastError());
                return 22;
            }
            port = ntohs(*((uint16_t*)buf_addr));
            break;
        }
        case SOCKS_REQUEST_TYPE_DOMAIN:
        {
            uint8_t domain_len = 0;
            res = recv(_socket, (char*)&domain_len, sizeof(domain_len), 0);
            if (res == -1)
            {
                logs::err("No domain size provided", WSAGetLastError());
                return 23;
            }
            char buf_addr[256] = { 0 };
            res = recv(_socket, buf_addr, domain_len, 0);
            if (res == -1)
            {
                logs::err("No domain content provided", WSAGetLastError());
                return 24;
            }
            addr = std::string(buf_addr);

            res = recv(_socket, buf_addr, 2, 0);
            if (res == -1)
            {
                logs::err("No port content provided", WSAGetLastError());
                return 25;
            }
            port = ntohs(*((uint16_t*)buf_addr));
            break;
        }
        default:
        {
            logs::err("Invalid request type : " + std::to_string(type));

            _send_response(SOCKET_RESPONSE_ADDRESS_NOT_SUPPORTED, INVALID_SOCKET);

            return 26;
        }
        }

        logs::info("CONNECT to " + addr + ":" + std::to_string(port));

        SOCKET remote_socket;
        DWORD remote_socket_status = 0;

        switch (type)
        {
        case SOCKS_REQUEST_TYPE_IPV4:
        {            
            remote_socket_status = _connect_socket_v4(&remote_socket, addr, port);
            break;
        }
        case SOCKS_REQUEST_TYPE_DOMAIN:
        {
            remote_socket_status = _connect_socket_domain(&remote_socket, addr, port);
            break;
        }
        case SOCKS_REQUEST_TYPE_IPV6:
        {           
            remote_socket_status = _connect_socket_v6(&remote_socket, addr, port);
            break;
        }          
        }

        if (!remote_socket_status)
        {
            _send_response(SOCKET_RESPONSE_SUCCEEDED, remote_socket);
            
            logs::debug("Tunneling data");
            
            auto tunnel_data_routine = [](SOCKET src, SOCKET dst) -> void {
                char buf[1024] = { 0 };

                fd_set readfds;
                int32_t nfds = (uint32_t) (max(src, dst) + 1);
                set_fds((int32_t)src, (int32_t)dst, &readfds);

                while (select(nfds, &readfds, 0, 0, 0) != SOCKET_ERROR)
                {
                    if (FD_ISSET(src, &readfds))
                    {
                        int recvd = recv(src, buf, sizeof(buf), 0);
                        if (recvd <= 0)
                            break;
                        send(dst, buf, recvd, 0);
                    }
                    if (FD_ISSET(dst, &readfds))
                    {
                        int recvd = recv(dst, buf, sizeof(buf), 0);
                        if (recvd <= 0)
                            break;
                        send(src, buf, recvd, 0);
                    }
                    set_fds((int32_t)src, (int32_t)dst, &readfds);
                }
            };

            std::thread client_thread(tunnel_data_routine, _socket, remote_socket);
            client_thread.join();

            shutdown(remote_socket, SD_BOTH);
            closesocket(remote_socket);

            logs::debug("End of tunnel");
        }
        else
        {
            switch (remote_socket_status)
            {
            case ENETUNREACH:
            {
                _send_response(SOCKET_RESPONSE_NETWORK_UNREACHABLE, remote_socket);
                break;
            }
            case EHOSTUNREACH:
            {
                _send_response(SOCKET_RESPONSE_HOST_UNREACHABLE, remote_socket);
                break;
            }
            case ECONNREFUSED:
            {
                _send_response(SOCKET_RESPONSE_REFUSED, remote_socket);
                break;
            }
            default:
            {
                _send_response(SOCKET_RESPONSE_SERVER_FAILURE, remote_socket);
            }
            }
        }

        break;
    }
    case SOCKS_COMMAND_BIND:
    {
        logs::err("Bind command not implemented yet");

        _send_response(SOCKET_RESPONSE_COMMAND_NOT_SUPPORTED, INVALID_SOCKET);

        return 27;
    }
    case SOCKS_COMMAND_UPD_ASSOCIATE:
    {
        logs::err("UDP associate command not implemented yet");

        _send_response(SOCKET_RESPONSE_COMMAND_NOT_SUPPORTED, INVALID_SOCKET);
        
        return 28;
    }
    default:
    {
        _send_response(SOCKET_RESPONSE_COMMAND_NOT_SUPPORTED, INVALID_SOCKET);
    }
    }

    return 0;
}

DWORD SocksClient::_send_response(uint8_t resp, SOCKET sock)
{
    char buf[10] = { 0 };
	memset(buf, 0, sizeof(buf));
    buf[0] = 5;
    buf[1] = resp;
    buf[2] = 0;
	buf[3] = 1; 
	
    logs::debug("Sending response : " + std::to_string(resp));

    return send(_socket, buf, 10, 0);
}

DWORD SocksClient::_connect_socket_v4(SOCKET * s, std::string addr, uint16_t port)
{
    *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*s == INVALID_SOCKET)
    {
        logs::err("socket (v4) failed", WSAGetLastError());
        return 29;
    }

    SOCKADDR_IN sin;
    inet_pton(AF_INET, addr.c_str(), &sin.sin_addr.s_addr);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    
    int conn_status = connect(*s, (SOCKADDR *)&sin, sizeof(sin));
    if (conn_status == SOCKET_ERROR)
    {
        return WSAGetLastError();
    }
    return 0;
}

DWORD SocksClient::_connect_socket_domain(SOCKET * s, std::string domain, uint16_t port)
{
    int status;
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof hints);

    if ((status = getaddrinfo(domain.c_str(), std::to_string(port).c_str(), &hints, &res)) != 0)
    {
        logs::err("getaddrinfo failed", status);
        return 30;
    }

    DWORD ret = 0;

    for (p = res; p != NULL; p = p->ai_next)
    {
        *s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (*s == INVALID_SOCKET)
        {
            logs::err("socket (domain) failed", WSAGetLastError());
            return 31;
        }
               
        status = connect(*s, p->ai_addr, (int)p->ai_addrlen);
        if (status == SOCKET_ERROR)
        {
            ret = WSAGetLastError();
        }
        else
        {
            ret = 0;
            break;
        }
    }

    freeaddrinfo(res);
  
    return ret;
}

DWORD SocksClient::_connect_socket_v6(SOCKET * s, std::string addr, uint16_t port)
{
    *s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == INVALID_SOCKET)
    {
        logs::err("socket (v6) failed", WSAGetLastError());
        return 32;
    }

    SOCKADDR_IN6 sin;
    inet_pton(AF_INET6, addr.c_str(), &sin.sin6_addr);
    sin.sin6_family = AF_INET6;
    sin.sin6_port = htons(port);
    
    int conn_status = connect(*s, (SOCKADDR *)&sin, sizeof(sin));
    if (conn_status == SOCKET_ERROR)
    {
        return WSAGetLastError();
    }
    return 0;
}

SocksClient::SocksClient(SOCKET socket)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    socklen_t len = sizeof(sin);

	u_long iMode = 0;
	int iResult = ioctlsocket(socket, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
	{
		logs::err("ioctlsocket failed", iResult);
        shutdown(_socket, SD_BOTH);
        closesocket(_socket);
        return;
	}

    _socket = socket;

    getsockname(socket, (struct sockaddr *)&sin, &len);
    
    _ip = utils::ip_to_str((const sockaddr*)&sin);
    _port = utils::get_port((const sockaddr*)&sin);

    _id = _count++;

    logs::debug("Connected client " + _ip + " (id:" + std::to_string(_id) + ")");
}

SocksClient::~SocksClient()
{
    if (_socket != INVALID_SOCKET) closesocket(_socket);

    logs::debug("Disconnected client " + _ip + " (id:" + std::to_string(_id) + ")");
}

void SocksClient::handle()
{
    if (_authenticate())
    {
        logs::debug("Client not authenticated (id:" + std::to_string(_id) + ")");
    }
    else
    {
        logs::debug("Client authenticated (id:" + std::to_string(_id) + ")");
        _exec_command();

        shutdown(_socket, SD_BOTH);
        closesocket(_socket);
    }
}
