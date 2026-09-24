#undef UNICODE  // for ANSI

// exclude legacy headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>    // should exclude winsocket 1.1

#include <winsock2.h>   // core network
#include <ws2tcpip.h>   // modern functions

#pragma comment(lib, "Ws2_32.lib")          // link library
// #pragma comment (lib, "Mswsock.lib")     // for AcceptEx

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
        std::cout << "WSAStartup 실패! 에러 코드 : " << iResult << std::endl;

        return 1;
    }

    std::cout << "Winsock 2.2 초기화 성공!" << std::endl;

    // 2. 주소 정보 준비 -----------------------------------------------

        // hints에 맞는 (내가원하는 규격의) 주소 구조체를 찾아서 addrResult에 넣어줘라
    struct addrinfo* addrResult = NULL;     // 주소체계, 포트번호, IP주소, ...
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));  // memset(0)

    hints.ai_family = AF_UNSPEC;        // Unspecified
    hints.ai_socktype = SOCK_STREAM;    // stream
    hints.ai_protocol = IPPROTO_TCP;

        // 상대방주소는 localhost로 테스트
    iResult = getaddrinfo("localhost", DEFAULT_PORT, &hints, &addrResult);
    if (iResult != 0)
    {
        std::cout << "getaddrinfo 실패 : " << iResult << std::endl;

        WSACleanup();

        return 1;
    }

        // connect 성공할때까지 받은 주소정보 루프
    SOCKET ConnectSocket = INVALID_SOCKET;
    for (ptr = addrResult; ptr != NULL; ptr = ptr->ai_next)
    {

    // 3. 소켓 생성 -------------------------------------------------------

        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); // 받은 addrResult의 규격대로 소켓 생성

        if (ConnectSocket == INVALID_SOCKET)
        {
            std::cout << "소켓 생성 실패 : " << WSAGetLastError() << std::endl;

            freeaddrinfo(addrResult);
            WSACleanup();

            return 1;
        }
        std::cout << "클라이언트 소켓 생성 성공!" << std::endl;

    // 4. 서버에 연결 시도 -----------------------------------------------------

        // 3way handshake 시작
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            std::cout << "서버 연결 실패! 에러 코드: " << WSAGetLastError() << std::endl;

            closesocket(ConnectSocket);

            // 원래는 목적지의 또다른 주소정보로 소켓을 만든후 connect하는 루프를 돌아야함
            ConnectSocket = INVALID_SOCKET;

            continue;
        }

        break;
    }

        // 다쓴 주소정보 해제
    freeaddrinfo(addrResult);

        // 최종적으로 연결에 실패했는지 확인
    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "서버에 접속할 수 없습니다! (서버가 켜져 있는지 확인하세요)" << std::endl;

        WSACleanup();

        return 1;
    }

    std::cout << "서버에 성공적으로 연결되었습니다! (connect 성공)" << std::endl;

    // 5. 서버에 데이터 전송 및 대기

    const char* sendbuf = "this is a test";
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];

        // 전송
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "send 실패! 에러: " << WSAGetLastError() << std::endl;

        closesocket(ConnectSocket);
        WSACleanup();

        return 1;
    }

    std::cout << "[송신 성공] 보낸 바이트: " << iResult << " (" << sendbuf << ")" << std::endl;

        // 대기
    std::cout << "서버의 답장을 기다리는 중..." << std::endl;

    do
    {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            std::cout << "[수신 성공] 서버로부터 받은 바이트: " << iResult << std::endl;
        else if (iResult == 0)
            std::cout << "서버가 전화를 정상적으로 끊었습니다. (통신 완료)" << std::endl;
        else
            std::cout << "recv 실패! 에러: " << WSAGetLastError() << std::endl;
    } while (iResult > 0);

    // 6. 소켓 연결 끊기

    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());

        closesocket(ConnectSocket);
        WSACleanup();

        return 1;
    }

    // 7. 뒷정리

    closesocket(ConnectSocket);
    WSACleanup();

    std::cout << "[Client] Winsock 환경 설정 성공!" << std::endl;

    return 0;
}