#undef UNICODE  // for ANSI

// exclude legacy headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>    // should exclude winsocket 1.1

#include <winsock2.h>   // core network
#include <ws2tcpip.h>   // modern functions

#include <iphlpapi.h>   // ip helper api

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
        std::cout << "WSAStartup 실패! 에러 코드: " << iResult << std::endl;
        return 1;
    }

    std::cout << "Winsock 2.2 초기화 성공!" << std::endl;

    // 2. 주소 정보 준비 -----------------------------------------

    struct addrinfo* addrResult = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));  // memset(0)

        // ai : adress information
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // datagram
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;        // bitmask for server

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrResult);
    if (iResult != 0)
    {
        std::cout << "getaddrinfo 실패 : " << iResult << std::endl;

        WSACleanup();

        return 1;
    }

    // 3. UDP 서버 소켓 생성 --------------------------------------

        // UDP는 리슨소켓 따로X, 소켓 하나로 데이터를 주고받음
    SOCKET ServerSocket = INVALID_SOCKET;
    ServerSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);

    if (ServerSocket == INVALID_SOCKET)
    {
        std::cout << "소켓 생성 실패! 에러 : " << WSAGetLastError() << std::endl;

        freeaddrinfo(addrResult);
        WSACleanup();

        return 1;
    }

    std::cout << "UDP 서버 소켓 생성 성공!" << std::endl;

    // 4. 소켓 바인딩 --------------------------------------------

    iResult = bind(ServerSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "소켓 바인딩 실패! 에러 : " << WSAGetLastError() << std::endl;

        freeaddrinfo(addrResult);
        closesocket(ServerSocket);
        WSACleanup();

        return 1;
    }

    freeaddrinfo(addrResult);   // 바인딩 이후 필요 없음

    std::cout << DEFAULT_PORT << " 포트에 바인딩 성공!" << std::endl;

    std::cout << DEFAULT_PORT << " 포트에서 클라이언트의 UDP 패킷 대기 중...\n";

    // 5. 데이터 주고받기 (에코) ----------------------------------

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iSendResult;

        // UDP는 연결이 없으므로, 편지를 보낸 상대방주소 담을 바구니 필수
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    std::cout << "데이터 수신 대기 (recvfrom)..." << std::endl;

        // 수신 : 패킷을 받으면서 보낸 사람의 주소(clientAddr)를 함께 알아냄
    iResult = recvfrom(ServerSocket, recvbuf, recvbuflen, 0, (sockaddr*)&clientAddr, &clientAddrLen);
    if (iResult > 0)
    {
        std::cout << "[수신] 받은 바이트 수 : " << iResult << std::endl;

            // 송신 : 알아낸 손님 주소(clientAddr)로 그대로 에코(sendto) 전송
        iSendResult = sendto(ServerSocket, recvbuf, iResult, 0, (sockaddr*)&clientAddr, clientAddrLen);
        if (iSendResult == SOCKET_ERROR)
        {
            std::cout << "sendto 실패! 에러 : " << WSAGetLastError() << std::endl;

            closesocket(ServerSocket);
            WSACleanup();

            return 1;
        }

        std::cout << "[송신] 에코 전송 완료 : " << iSendResult << " 바이트" << std::endl;
    }
    else
    {
        std::cout << "recvfrom 실패! 에러 : " << WSAGetLastError() << std::endl;

        closesocket(ServerSocket);
        WSACleanup();

        return 1;
    }

    // 6. 뒷정리 ------------------------------------------------

    closesocket(ServerSocket);
    WSACleanup();

    std::cout << "[Server] UDP 에코 서버 정상 종료!" << std::endl;

    return 0;
}