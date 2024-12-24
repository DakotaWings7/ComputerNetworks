#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <algorithm>

// Подключение библиотеки WinSock
#pragma comment(lib, "ws2_32.lib")

const int PORT = 8000;
const int BUFFER_SIZE = 1024;

std::vector<SOCKET> client_sockets;
std::atomic<bool> server_running(true);

void HandleClientMessage(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        std::cout << "Клиент отключился. Сокет: " << client_socket << std::endl;
        closesocket(client_socket);
        client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
        return;
    }

    std::string message(buffer, bytes_received);
    std::cout << "Сообщение от клиента " << client_socket << ": " << message << std::endl;

    for (SOCKET sock : client_sockets) {
        if (sock != client_socket) {
            send(sock, message.c_str(), message.size(), 0);
        }
    }
}

void SignalHandler() {
    server_running = false;
    for (SOCKET sock : client_sockets) closesocket(sock);
    client_sockets.clear();
    WSACleanup();
    exit(0);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации WinSock" << std::endl;
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR ||
        listen(server_socket, 10) == SOCKET_ERROR) {
        std::cerr << "Ошибка привязки или прослушивания" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Сервер запущен на порту " << PORT << std::endl;

    while (server_running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);

        SOCKET max_fd = server_socket;
        for (SOCKET client_sock : client_sockets) {
            FD_SET(client_sock, &read_fds);
            if (client_sock > max_fd) max_fd = client_sock;
        }

        if (select(0, &read_fds, nullptr, nullptr, nullptr) < 0) continue;

        if (FD_ISSET(server_socket, &read_fds)) {
            sockaddr_in client_addr{};
            int client_size = sizeof(client_addr);
            SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
            if (client_socket == INVALID_SOCKET) continue;

            client_sockets.push_back(client_socket);
            std::cout << "Новый клиент: " << client_socket << std::endl;
        }

        for (SOCKET client_socket : client_sockets) {
            if (FD_ISSET(client_socket, &read_fds)) HandleClientMessage(client_socket);
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
