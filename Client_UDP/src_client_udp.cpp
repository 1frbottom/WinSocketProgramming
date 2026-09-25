#undef UNICODE  // for ANSI

// exclude legacy headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>    // should exclude winsocket 1.1

#include <winsock2.h>   // core network
#include <ws2tcpip.h>   // modern functions

#pragma comment(lib, "Ws2_32.lib")  // link library

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512      // buffer length

//

#include <iostream>



int main()
{
    // 1. Winsock 초기화 ----------------------------------------

    WSADATA wsaData;    // winsocket async data, 라이브러리 세팅 확인용

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // using WS 2.2
    if (iResult != 0)
    {
        std::cout << "WSAStartup 실패! 에러 코드 : " << iResult << std::endl;

        return 1;
    }

    std::cout << "Winsock 2.2 초기화 성공!" << std::endl;

    // 2. 목적지(서버) 주소 정보 준비 ------------------------------

    struct addrinfo* addrResult = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));  // memset(0)

    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // datagram
    hints.ai_protocol = IPPROTO_UDP;

    iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &addrResult);
    if (iResult != 0)
    {
        std::cout << "getaddrinfo 실패 : " << iResult << std::endl;

        WSACleanup();

        return 1;
    }

    // 3. 소켓 생성 ---------------------------------------------

    SOCKET ClientSocket = INVALID_SOCKET;
    ClientSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);

    if (ClientSocket == INVALID_SOCKET)
    {
        std::cout << "소켓 생성 실패 : " << WSAGetLastError() << std::endl;

        freeaddrinfo(addrResult);
        WSACleanup();

        return 1;
    }

    std::cout << "UDP 클라이언트 소켓 생성 성공!" << std::endl;

    // 4. 서버에 데이터 전송 및 답장 수신 ---------------------------

    const char* sendbuf = "this is a test (UDP)";
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];

        // 서버 주소(addrResult->ai_addr)를 찍어서 전송
    iResult = sendto(ClientSocket, sendbuf, (int)strlen(sendbuf), 0,
        addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "sendto 실패! 에러: " << WSAGetLastError() << std::endl;

        freeaddrinfo(addrResult);
        closesocket(ClientSocket);
        WSACleanup();

        return 1;
    }

    std::cout << "[송신 성공] 서버로 보낸 바이트: " << iResult << " (" << sendbuf << ")" << std::endl;

        // 주소 정보는 이제 다 썼으니 해제
    freeaddrinfo(addrResult);

        // 서버가 에코로 되돌려준 답장 수신
    std::cout << "서버의 답장을 기다리는 중..." << std::endl;

    sockaddr_in fromAddr;
    int fromAddrLen = sizeof(fromAddr);

    iResult = recvfrom(ClientSocket, recvbuf, recvbuflen, 0, (sockaddr*)&fromAddr, &fromAddrLen);
    if (iResult > 0)
    {
        // 문자열 출력을 위해 끝에 널 삽입
        recvbuf[iResult] = '\0';

        std::cout << "[수신 성공] 서버로부터 받은 바이트: " << iResult << " (" << recvbuf << ")" << std::endl;
    }
    else
        std::cout << "recvfrom 실패! 에러: " << WSAGetLastError() << std::endl;

    // 5. 뒷정리 ------------------------------------------------

    closesocket(ClientSocket);
    WSACleanup();

    std::cout << "[Client] UDP 통신 완료 및 정상 종료!" << std::endl;

    return 0;
}