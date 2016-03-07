#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib") 

#define MAX_RECV_BUFFSIZE 128
int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        return -1;
    }

    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == ClientSocket)
    {
        printf("Creat socket fail! error code = %u", GetLastError());
    }

    int nResult = 0;
    SOCKADDR_IN  addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //竟然不知道
    addr.sin_port = 7878;
    nResult = bind(ClientSocket, (LPSOCKADDR)&addr, sizeof(SOCKADDR_IN));
    if (0 != nResult)
    {
        printf("bind fail!  %d\n", GetLastError());
    }

    HANDLE hEvent = WSACreateEvent();

    nResult = WSAEventSelect(ClientSocket, hEvent, FD_CONNECT | FD_CLOSE | FD_READ);
    if (0 != nResult)
    {
        printf("WSAEventSelect() fail!  %d\n", GetLastError());
    }

    SOCKADDR_IN  ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //竟然不知道
    ServerAddr.sin_port = 8989;
    connect(ClientSocket, (sockaddr*)&ServerAddr, sizeof(SOCKADDR_IN));

    while (true)
    {
        int ret = WSAWaitForMultipleEvents(1, &hEvent, true, 1000, false);
        if (WSA_WAIT_FAILED == ret || WSA_WAIT_TIMEOUT == ret)
        {
            continue;
        }
        else
        {
            WSANETWORKEVENTS NetEvent;
            WSAEnumNetworkEvents(ClientSocket, hEvent, &NetEvent);
            if (NetEvent.lNetworkEvents & FD_CONNECT)
            {
                char cBuff[MAX_RECV_BUFFSIZE] = { 0 };
                printf("与服务器连接成功！\n");
            }
            if (NetEvent.lNetworkEvents & FD_READ)
            {
                printf("wait recv content....\n");
                char cBuff[MAX_RECV_BUFFSIZE] = { 0 };
                int nRecvSize = recv(ClientSocket, cBuff, MAX_RECV_BUFFSIZE, 0);
                if (nRecvSize < 0)
                {
                    printf("recv fail！\n");
                }
                else
                {
                    printf("recv content : %s\n", cBuff);
                }
            }
            else if (NetEvent.lNetworkEvents & FD_CLOSE)
            {
                printf("关闭客户端！\n");
            }

        }

    }

    return 0;
}