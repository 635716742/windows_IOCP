#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib") 
#define MAX_SEND_BUFFSIZE 128
int main(int argc, char** argv)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return -1;
    }

    int TotalClientNum = 0;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    int nResult = 0;
    if (INVALID_SOCKET == sock)
    {
        printf("Create sock fail!\n");
    }

    SOCKADDR_IN  addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //竟然不知道
    addr.sin_port = 8989;
    nResult = bind(sock, (LPSOCKADDR)&addr, sizeof(SOCKADDR_IN));
    if (0 != nResult)
    {
        printf("bind fail!  %d\n", GetLastError());
    }

    HANDLE hEvent = WSACreateEvent();

    nResult = WSAEventSelect(sock, hEvent, FD_ACCEPT | FD_CLOSE);
    if (0 != nResult)
    {
        printf("WSAEventSelect() fail!  %d\n", GetLastError());
    }

    nResult = listen(sock, 5);
    if (0 != nResult)
    {
        printf("listen() fail!  %d\n", GetLastError());
    }

    WSAEVENT arrEvent[WSA_MAXIMUM_WAIT_EVENTS] = {0};
    SOCKET   arrSock[WSA_MAXIMUM_WAIT_EVENTS] = {0};
    arrEvent[TotalClientNum] = hEvent;
    arrSock[TotalClientNum] = sock;
    unsigned int nClientNumber = 1;

    while (true)
    {
        int nIndex = WSAWaitForMultipleEvents(1, arrEvent, false, WSA_INFINITE, false);
        nIndex = nIndex - WSA_WAIT_EVENT_0;
        for (int i = nIndex; i < 1; i++)
        {
            int ret = WSAWaitForMultipleEvents(1, &arrEvent[i], true, 1000, false);
            if (WSA_WAIT_FAILED == ret || WSA_WAIT_TIMEOUT == ret)
            {
                continue;
            }
            else
            {
                WSANETWORKEVENTS NetEvent;
                WSAEnumNetworkEvents(arrSock[i], arrEvent[i], &NetEvent);
                if (NetEvent.lNetworkEvents & FD_ACCEPT)
                {
                    if (nClientNumber > 63)
                    {
                        printf("连接数达到最大！\n");
                        break;
                    }
                    char cBuff[MAX_SEND_BUFFSIZE] = { "123456789ABCDEF" };
                    SOCKADDR_IN  ClientAddr;
                    int len = sizeof(SOCKADDR_IN);
                    arrSock[nClientNumber] = accept(arrSock[0], (sockaddr*)&ClientAddr, &len);
                    if (INVALID_SOCKET == arrSock[nClientNumber])
                    {
                        printf("client accept fail! core = %u\n", GetLastError());
                        break;
                    }
                    printf("monitor FD_ACCEPT,client info: IP = %s, port = %d\n", inet_ntoa(ClientAddr.sin_addr), ClientAddr.sin_port);
                    arrEvent[nClientNumber] = WSACreateEvent();

                    nResult = WSAEventSelect(arrSock[nClientNumber], arrEvent[nClientNumber], FD_WRITE | FD_READ | FD_CLOSE);
                    if (0 != nResult)
                    {
                        printf("WSAEventSelect() client fail!  %d\n", GetLastError());
                    }

                    if (send(arrSock[nClientNumber], cBuff, MAX_SEND_BUFFSIZE, 0) < 0)
                    {
                        printf("send content fail! core = %u\n", GetLastError());
                    }

                    //memset(cBuff, 0 , MAX_SEND_BUFFSIZE);
                    //memcpy(cBuff, "ssssssssssss", sizeof("ssssssssssss"));
                    //send(arrSock[nClientNumber], cBuff, MAX_SEND_BUFFSIZE, 0);
                    //if (send(arrSock[nClientNumber], cBuff, MAX_SEND_BUFFSIZE, 0) < 0)
                    //{
                    //    printf("send content fail! core = %u\n", GetLastError());
                    //}

                    nClientNumber++;
                }
                else if (NetEvent.lNetworkEvents & FD_READ)
                {
                    printf("monitor FD_READ\n");
                }
                else if (NetEvent.lNetworkEvents & FD_WRITE)
                {
                    printf("monitor FD_WRITE\n");
                }
                else if (NetEvent.lNetworkEvents & FD_CLOSE)
                {
                    printf("monitor FD_CLOSE\n");
                }
            }
        }
    }

    return 0;
}