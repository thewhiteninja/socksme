#include "socks_server.h"
#include "socks_client.h"
#include "utils.h"

#pragma comment(lib, "ws2_32.lib")

SOCKET server_socket = INVALID_SOCKET;

SocksServer::SocksServer()
{
    logs::debug("Creating socks server");
    _is_ready = false;

    _local_ip = Options::bind_ip;
    _local_port = Options::bind_port;

    int res = 0;

    WSADATA WSAData;
    res = WSAStartup(MAKEWORD(2, 0), &WSAData);
    if (res != 0)
    {
        logs::err("WSAStartup failed", res);
        return;
    }

    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == INVALID_SOCKET)
    {
        logs::err("socket failed", WSAGetLastError());
        return;
    }

    int optVal = 1;
    res = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal));
    if (res == SOCKET_ERROR)
    {
        logs::err("setsockopt failed", WSAGetLastError());
        return;
    }

    u_long iMode = 1;
    int iResult = ioctlsocket(_socket, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
    {
        logs::err("ioctlsocket failed", iResult);
    }

    SOCKADDR_IN sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(_local_port);
    res = inet_pton(AF_INET, _local_ip.c_str(), &sin.sin_addr.s_addr);
    if (res != 1)
    {
        logs::err("inet_pton failed", res);
        return;
    }

    res = bind(_socket, (SOCKADDR *)&sin, sizeof(sin));
    if (res == SOCKET_ERROR)
    {
        logs::err("bind failed", res);
        return;
    }

    res = listen(_socket, Options::max_conn);
    if (res != 0)
    {
        logs::err("listen failed", res);
        return;
    }

    server_socket = _socket;

    _is_ready = true;
}

SocksServer::~SocksServer()
{
    if (_socket != INVALID_SOCKET) closesocket(_socket);
    WSACleanup();
}

BOOL WINAPI ctrl_handler(DWORD type)
{
    if (type == CTRL_C_EVENT)
    {
        logs::debug("Ctrl+C received. Stopping socks server.");

        closesocket(server_socket);
        server_socket = INVALID_SOCKET;

        return TRUE;
    }
    return FALSE;
}

void SocksServer::run()
{
    logs::info("Socks server running on " + _local_ip + ":" + std::to_string(_local_port));

    SetConsoleCtrlHandler(ctrl_handler, TRUE);

    fd_set server_set;
    FD_ZERO(&server_set);
    FD_SET(_socket, &server_set);

    while (true)
    {
        int rc = select((int32_t)(_socket + 1), &server_set, NULL, NULL, NULL);
        if (rc < 0)
        {
            int err = WSAGetLastError();
            if (err != WSAENOTSOCK || server_socket != INVALID_SOCKET)
            {
                logs::err("select failedd", err);                
            }
            break;
        }
        if (rc == 0)
        {
            continue;
        }

        if (FD_ISSET(_socket, &server_set))
        {
            SOCKET client_socket = 0;
            do
            {
                client_socket = accept(_socket, 0, 0);
                if (client_socket == INVALID_SOCKET)
                {
                    int err = WSAGetLastError();
                    if (err != WSAEWOULDBLOCK)
                    {
                        if (err != WSAENOTSOCK || server_socket != INVALID_SOCKET)
                        {
                            logs::err("Accept failed", WSAGetLastError());
                            return;
                        }
                    }
                    break;
                }
                else
                {
                    auto new_client_routine = [](LPVOID param2) -> DWORD {
                        std::shared_ptr<SocksClient> client = std::make_shared<SocksClient>((SOCKET)param2);
                        client->handle();
                        return TRUE;
                    };

                    CreateThread(0, 0, new_client_routine, (LPVOID)client_socket, 0, 0);
                }
            } while (client_socket != SOCKET_ERROR);
        }
    }

    logs::info("Socks server stopped");

    Options::log_level = loglevel::NONE;
}
