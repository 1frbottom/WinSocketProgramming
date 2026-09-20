#undef UNICODE  // for ANSI

// exclude legacy headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>    // should exclude winsocket 1.1

#include <winsock2.h>   // core network
#include <ws2tcpip.h>   // modern functions

#include <iphlpapi.h>   // ip helper api

#pragma comment(lib, "Ws2_32.lib")          // link library
// #pragma comment (lib, "Mswsock.lib")     // for AcceptEx

//

#include <iostream>

int main()
{
    WSADATA wsaData;    // winsocket async data

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // using WS 2.2
    if (iResult != 0)
    {
        std::cout << "WSAStartup 실패! 에러 코드: " << iResult << std::endl;

        return 1;
    }

    std::cout << "Winsock 2.2 초기화 성공!" << std::endl;







    WSACleanup();

    std::cout << "Winsock 환경 설정 성공!" << std::endl;

    return 0;
}