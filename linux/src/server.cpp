#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unordered_map>
#include <ctime>

constexpr int PORT = 5555;
constexpr size_t BUFFER_SIZE = 1500;
constexpr int CLIENT_TIMEOUT_SEC = 60;

struct ClientSession {
    sockaddr_in addr;
    time_t last_activity;
};

// Прототип функции из tun.cpp
int tun_alloc(const char* devname);

std::string make_client_key(const sockaddr_in& addr) {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str));
    return std::string(ip_str) + ":" + std::to_string(ntohs(addr.sin_port));
}

int main(int argc, char* argv[]) {
    const char* listen_ip = "0.0.0.0";  

    if (argc > 1) {
        listen_ip = argv[1];
    }

    int tun_fd = tun_alloc("tun0");
    if (tun_fd < 0) {
        std::cerr << "[-] Failed to create TUN interface\n";
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0) {
        std::cerr << "Invalid listen IP address\n";
        return 1;
    }

    if (bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        return 1;
    }

    std::unordered_map<std::string, ClientSession> clients;

    pollfd fds[2];
    fds[0].fd = tun_fd;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    char buffer[BUFFER_SIZE];

    std::cout << "[Server] Running. Waiting for TUN and UDP events...\n";

    while (true) {
        int nready = poll(fds, 2, 1000);
        if (nready < 0) {
            perror("poll");
            break;
        }

        time_t now = time(nullptr);

        // Удаляем неактивных клиентов
        for(auto it = clients.begin(); it != clients.end();) {
            if (now - it->second.last_activity > CLIENT_TIMEOUT_SEC) {
                std::cout << "[Server] Client timeout: " << it->first << "\n";
                it = clients.erase(it);
            } else {
                ++it;
            }
        }

        if (nready == 0)
            continue;

      
        if (fds[0].revents & POLLIN) {
            ssize_t nread = read(tun_fd, buffer, sizeof(buffer));
            if (nread < 0) {
                perror("read tun");
                break;
            }

            for (auto& [key, session] : clients) {
                ssize_t n = sendto(sockfd, buffer, nread, 0,
                                   (sockaddr*)&session.addr, sizeof(session.addr));
                if (n < 0) perror("sendto");
            }

            std::cout << "[Server] Forwarded " << nread << " bytes from TUN to clients\n";
        }

        // 2) Данные из UDP — пишем в TUN
        if (fds[1].revents & POLLIN) {
            sockaddr_in cliaddr{};
            socklen_t clilen = sizeof(cliaddr);

            ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                 (sockaddr*)&cliaddr, &clilen);
            if (n < 0) {
                perror("recvfrom");
                continue;
            }

            std::string key = make_client_key(cliaddr);
            auto& session = clients[key];
            session.addr = cliaddr;
            session.last_activity = now;

            ssize_t written = write(tun_fd, buffer, n);
            if (written < 0) {
                perror("write tun");
            }

            std::cout << "[Server] Received " << n << " bytes from " << key << " and wrote to TUN\n";
        }
    }

    close(tun_fd);
    close(sockfd);
    return 0;
}