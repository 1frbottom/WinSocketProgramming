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

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512      // buffer length

//

#include <iostream>



int main()
{
    // 1. Winsock 초기화 ----------------------------------------

    WSADATA wsaData;    // winsocket async data, 라이브러리 세팅 확인용

        // iResult는 윈소켓 주요 api함수들이 반환하는 성공여부 홀더로써 계속 재사용
        // 메모리에 Ws2_32.lib을 올림
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // using WS 2.2
    if (iResult != 0)
    {
        std::cout << "WSAStartup 실패! 에러 코드: " << iResult << std::endl;

        return 1;
    }

    std::cout << "Winsock 2.2 초기화 성공!" << std::endl;

    // 2. 주소 정보 준비 -----------------------------------------
        
        // hints에 맞는 (내가원하는 규격의) 주소 구조체를 찾아서 addrResult에 넣어줘라
    struct addrinfo* addrResult = NULL;     // 주소체계, 포트번호, IP주소, ...
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));  // memset(0)

        // ai : adress information
    hints.ai_family = AF_INET;          // family : 규격 단위
    hints.ai_socktype = SOCK_STREAM;    // stream
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;        // bitmask for server

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrResult);
    if (iResult != 0)
    {
        std::cout << "getaddrinfo 실패 : " << iResult << std::endl;

        WSACleanup();

        return 1;
    }

    // 3. 리슨 소켓 생성 ------------------------------------------

    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol); // 받은 addrResult의 규격대로 소켓 생성

    if (ListenSocket == INVALID_SOCKET)
    {
        std::cout << "소켓 생성 실패! 에러 : " << WSAGetLastError() << std::endl;

        freeaddrinfo(addrResult);
        WSACleanup();

        return 1;
    }

    std::cout << "서버 소켓(ListenSocket) 생성 성공!" << std::endl;

    // 4. 리슨 소켓 바인딩 ------------------------------------------------

    iResult = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "소켓 바인딩 실패! 에러 : " << WSAGetLastError() << std::endl;

        freeaddrinfo(addrResult);
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    freeaddrinfo(addrResult);   // 바인딩이후 필요없음

    std::cout << DEFAULT_PORT << " 포트에 바인딩 성공!" << std::endl;

    // 5. 리슨소켓의 수신 대기 ----------------------------------------------------

        // 리슨 : 연결요청 쌓아둘 backlog queue 생성, accept()로 꺼내기
        // 리슨소켓은 수신 요청 관리 전용, 실제 수신은 전용 소켓을 새로 만듬
    iResult = listen(ListenSocket, SOMAXCONN);  // Socket Maximum Connections
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "리슨 실패! 에러 : " << WSAGetLastError() << std::endl;

        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    std::cout << DEFAULT_PORT << " 포트에서 클라이언트를 기다리는 중...\n";

    // 6. 수신 수락 ---------------------------------------------------------

    SOCKET ClientSocket = INVALID_SOCKET;
    ClientSocket = accept(ListenSocket, NULL, NULL);    // Blocking 방식 : 클라이언트가 connect 완료하기 전까지, 즉 handshake 끝나기 전까지 코드(스레드)는 여기서 멈춰서 기다림
                                                        // 실제로는 논블로킹 + 이벤트 감시 주로 사용
                                                        // 게임에서는 목적에 맞게 RUDP(이동조준), 윈도우IOCP+AcceptEx(대규모게임로직), 리눅스도커-프레임워크(플랫폼,매칭)

    if (ClientSocket == INVALID_SOCKET)
    {
        std::cout << "accept 실패! 에러 : " << WSAGetLastError() << std::endl;

        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    std::cout << "클라이언트가 성공적으로 접속했습니다!" << std::endl;
    
    closesocket(ListenSocket);  // 테스트용이므로 일단 닫아놓기

    // 7. 데이터 주고받기 ------------------------------------------------------

    char recvbuf[DEFAULT_BUFLEN];       // receive buffer
    int recvbuflen = DEFAULT_BUFLEN;
    int iSendResult;

    std::cout << "데이터 수신 대기..." << std::endl;

    do
    {
        // 손님의 말을 기다림 (데이터 올 때까지 멈춤)
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);   // accept와 동일하게 기본은 블로킹
        
        if (iResult > 0)        // 성공, 반환값은 받은 바이트수
        {
            std::cout << "[수신] 받은 바이트 수 : " << iResult << std::endl;

            // send, 테스트용 에코
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR)
            {
                std::cout << "send 실패! 에러 : " << WSAGetLastError() << std::endl;

                closesocket(ClientSocket);
                WSACleanup();

                return 1;
            }

            std::cout << "[송신] 에코 전송 완료 : " << iSendResult << " 바이트" << std::endl;
        }
        else if (iResult == 0)  // 종료
            std::cout << "손님이 연결을 종료했습니다. (정상 종료)" << std::endl;
        else                    // 에러
        {
            std::cout << "recv 실패! 에러 : " << WSAGetLastError() << std::endl;

            closesocket(ClientSocket);
            WSACleanup();

            return 1;
        }
    } while (iResult > 0);

    // 8. 소켓 연결 끊기 -----------------------------------------------------

    // shutdown : 버퍼에 남은 데이터들 내보내면서 끝에 FIN 붙임
    // closesocket : 끝난걸로 간주하고 자원(소켓) 정리
    iResult = shutdown(ClientSocket, SD_SEND);      // ShutDown_SEND
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "shutdown 실패 : " << WSAGetLastError() << std::endl;

        closesocket(ClientSocket);
        WSACleanup();

        return 1;
    }

    // 9. 뒷정리 ----------------------------------------------------------------

    closesocket(ClientSocket);
    WSACleanup();               // 모듈 반납

    std::cout << "[Server] Winsock 환경 설정 성공!" << std::endl;

    return 0;
}