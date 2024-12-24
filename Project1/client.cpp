#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

// Подключение библиотеки WinSock
#pragma comment(lib, "ws2_32.lib")

const int PORT = 8000;
const int BUFFER_SIZE = 1024;

bool client_running = true;

void ReceiveMessages(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    while (client_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Соединение с сервером разорвано." << std::endl;
            client_running = false;
            break;
        }
        std::cout << "Сервер: " << buffer << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации WinSock" << std::endl;
        return 1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0 ||
        connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка подключения к серверу" << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Подключено к серверу." << std::endl;

    std::thread receive_thread(ReceiveMessages, client_socket);

    std::string message;
    while (client_running) {
        std::getline(std::cin, message);
        if (message == "/exit") {
            client_running = false;
            break;
        }

        if (send(client_socket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
            std::cerr << "Ошибка отправки сообщения" << std::endl;
            client_running = false;
            break;
        }
    }

    receive_thread.join();
    closesocket(client_socket);
    WSACleanup();
    std::cout << "Клиент завершил работу." << std::endl;
    return 0;
}
