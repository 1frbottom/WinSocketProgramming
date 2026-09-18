// exclude legacy headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>    // should exclude winsocket 1.1

#include <winsock2.h>   // core network
#include <ws2tcpip.h>   // modern functions

#include <iphlpapi.h>   // ip helper api

#pragma comment(lib, "Ws2_32.lib")  // link library

//

#define DEFAULT_PORT "27015"

//

#include <iostream>

int main()
{
    // 1. Winsock 초기화 ----------------------------------------

    WSADATA wsaData;    // winsocket async data, 라이브러리 세팅 확인용

        // 메모리에 Ws2_32.lib을 올림
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // using WS 2.2
    if (iResult != 0)
    {
        std::cout << "WSAStartup 실패! 에러 코드: " << iResult << std::endl;

        return 1;
    }

    std::cout << "Winsock 2.2 초기화 성공!" << std::endl;

    // 2. 주소 정보 준비 -----------------------------------------
        
        // hints에 맞는 주소를 찾아서 result에 넣어줘라
    struct addrinfo* addrResult = NULL;     // 주소체계, 포트번호, IP주소, ...
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));  // memset(0)

        // ai : adress information
    hints.ai_family = AF_INET;          // family : 규격 단위
    hints.ai_socktype = SOCK_STREAM;    // stream
    hints.ai_protocol = IPPROTO_TCP;    // protocol
    hints.ai_flags = AI_PASSIVE;        // bitmask for server

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrResult);
    if (iResult != 0)
    {
        std::cout << "getaddrinfo 실패 : " << iResult << std::endl;

        WSACleanup();

        return 1;
    }

    // 3. 서버 소켓 생성 ------------------------------------------

    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);

    if (ListenSocket == INVALID_SOCKET)
    {
        std::cout << "소켓 생성 실패! 에러 : " << WSAGetLastError()<< std::endl;

        freeaddrinfo(addrResult);
        WSACleanup();

        return 1;
    }

    std::cout << "서버 소켓(ListenSocket) 생성 성공!" << std::endl;






    // 뒷정리 ------------------------------------------------

    closesocket(ListenSocket);
    freeaddrinfo(addrResult);
    WSACleanup();               // 모듈 반납

    std::cout << "Winsock 환경 설정 성공!" << std::endl;

    return 0;
}