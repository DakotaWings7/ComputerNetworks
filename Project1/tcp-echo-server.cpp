#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

// Подключение библиотеки WinSock
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27015

void handle_client(SOCKET client_socket) {
    char buffer[DEFAULT_BUFLEN];
    int bytes_received;

    std::cout << "Новый клиент подключен: " << client_socket << "\n";

    while ((bytes_received = recv(client_socket, buffer, DEFAULT_BUFLEN, 0)) > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "Сообщение от клиента " << client_socket << ": " << buffer << "\n";

        if (strcmp(buffer, "exit") == 0) {
            std::cout << "Клиент " << client_socket << " завершил соединение.\n";
            break;
        }

        send(client_socket, buffer, bytes_received, 0);
    }

    if (bytes_received == 0) {
        std::cout << "Клиент " << client_socket << " отключился.\n";
    }
    else if (bytes_received == SOCKET_ERROR) {
        std::cerr << "Ошибка приёма данных от клиента " << client_socket << "\n";
    }

    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    SOCKET server_sock, client_sock;
    sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    // Инициализация WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации WinSock\n";
        return 1;
    }

    // Создание серверного сокета
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        std::cerr << "Ошибка создания серверного сокета\n";
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    // Привязка сокета
    if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка привязки сокета\n";
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    // Прослушивание входящих подключений
    if (listen(server_sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Ошибка перехода в режим прослушивания\n";
        closesocket(server_sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Сервер запущен и слушает порт " << DEFAULT_PORT << "\n";

    while (true) {
        client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == INVALID_SOCKET) {
            std::cerr << "Ошибка принятия подключения\n";
            continue;
        }

        std::thread client_thread(handle_client, client_sock);
        client_thread.detach();
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}
