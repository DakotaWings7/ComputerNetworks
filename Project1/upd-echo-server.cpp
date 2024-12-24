#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

constexpr int THREAD_POOL_SIZE = 10;

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return !tasks.empty() || stop; });

                        if (stop && tasks.empty())
                            return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task();
                }
                });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }

        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

    template<typename F, typename... Args>
    void NewTask(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(task);
        }

        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};

void Echo(SOCKET socket_fd, const std::string& message, const sockaddr_in& to) {
    sendto(socket_fd, message.c_str(), message.size(), 0,
        (sockaddr*)&to, sizeof(to));
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    SOCKET listen_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "UDP Echo Server is running on port 8080..." << std::endl;

    ThreadPool pool(THREAD_POOL_SIZE);

    while (true) {
        char buffer[1024];
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        int bytes_received = recvfrom(listen_socket, buffer, sizeof(buffer) - 1, 0,
            (sockaddr*)&client_addr, &client_addr_len);

        if (bytes_received == SOCKET_ERROR) {
            std::cerr << "Failed to receive data: " << WSAGetLastError() << std::endl;
            continue;
        }

        buffer[bytes_received] = '\0';

        pool.NewTask(Echo, listen_socket, std::string(buffer), client_addr);
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}
