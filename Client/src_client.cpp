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

#define DEFAULT_PORT "20715"

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

    // 3. 소켓 생성 -------------------------------------------------------

    ptr = addrResult; // 조회된 첫 번째 주소 사용

    SOCKET ConnectSocket = INVALID_SOCKET;
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); // 받은 addrResult의 규격대로 소켓 생성

    if (ConnectSocket == INVALID_SOCKET)
    {
        std::cout << "소켓 생성 실패 : " << WSAGetLastError() << std::endl;

        freeaddrinfo(addrResult);
        WSACleanup();

        return 1;
    }
    std::cout << "클라이언트 소켓 생성 성공!" << std::endl;

    // 4. -----------------------------------------------------


    // 마무리

    WSACleanup();

    std::cout << "Winsock 환경 설정 성공!" << std::endl;

    return 0;
}