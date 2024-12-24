#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    std::cout << "Enter your message (type 'exit' to quit):" << std::endl;

    while (true) {
        std::string message;
        std::cout << "> ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        int sent_bytes = sendto(client_socket, message.c_str(), message.size(), 0,
            (sockaddr*)&server_addr, sizeof(server_addr));
        if (sent_bytes == SOCKET_ERROR) {
            std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
            continue;
        }

        char buffer[1024];
        sockaddr_in from_addr;
        int from_len = sizeof(from_addr);

        int received_bytes = recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0,
            (sockaddr*)&from_addr, &from_len);
        if (received_bytes == SOCKET_ERROR) {
            std::cerr << "Failed to receive response: " << WSAGetLastError() << std::endl;
            continue;
        }

        buffer[received_bytes] = '\0';
        std::cout << "Server response: " << buffer << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}