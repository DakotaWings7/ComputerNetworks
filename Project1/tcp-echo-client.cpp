#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

// Подключение библиотеки WinSock
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27015

int main() {
    WSADATA wsaData;
    SOCKET client_sock;
    sockaddr_in server_addr;
    char buffer[DEFAULT_BUFLEN];
    int bytes_received;

    // Инициализация WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации WinSock" << std::endl;
        return 1;
    }

    // Создание сокета
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Подключение к серверу
    if (connect(client_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка подключения к серверу" << std::endl;
        closesocket(client_sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Подключено к серверу.\n";

    while (true) {
        std::cout << "Введите сообщение: ";
        std::cin.getline(buffer, DEFAULT_BUFLEN);

        if (send(client_sock, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            std::cerr << "Ошибка отправки данных" << std::endl;
            break;
        }

        if (strcmp(buffer, "exit") == 0) {
            std::cout << "Завершение соединения.\n";
            break;
        }

        bytes_received = recv(client_sock, buffer, DEFAULT_BUFLEN, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "Ответ сервера: " << buffer << "\n";
        }
        else if (bytes_received == 0) {
            std::cout << "Сервер закрыл соединение.\n";
            break;
        }
        else {
            std::cerr << "Ошибка приёма данных\n";
            break;
        }
    }

    closesocket(client_sock);
    WSACleanup();
    return 0;
}
