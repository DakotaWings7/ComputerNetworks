#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

int main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET sockfd = INVALID_SOCKET;
    struct addrinfo hints, * res, * p;
    int status;
    const char* sendbuf = "GET / HTTP/1.1\r\nHost: pmk.tversu.ru\r\nUser-Agent: Mozilla/5.0\r\nConnection: close\r\n\r\n";
    char recvbuf[DEFAULT_BUFLEN];
    int numbytes;

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    if (argc != 2) {
        fprintf(stderr, "usage: %s server-name\n", argv[0]);
        WSACleanup();
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        WSACleanup();
        return 2;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
            fprintf(stderr, "Socket error: %d\n", WSAGetLastError());
            continue;
        }

        if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sockfd);
            fprintf(stderr, "Connect error: %d\n", WSAGetLastError());
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to connect\n");
        freeaddrinfo(res);
        WSACleanup();
        return 2;
    }

    freeaddrinfo(res);

    if (send(sockfd, sendbuf, (int)strlen(sendbuf), 0) == SOCKET_ERROR) {
        fprintf(stderr, "Send error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    while ((numbytes = recv(sockfd, recvbuf, DEFAULT_BUFLEN - 1, 0)) > 0) {
        recvbuf[numbytes] = '\0';
        printf("%s", recvbuf);
    }

    if (numbytes == SOCKET_ERROR) {
        fprintf(stderr, "Recv error: %d\n", WSAGetLastError());
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
