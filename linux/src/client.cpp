#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

constexpr int PORT = 5555;
constexpr size_t BUFFER_SIZE = 1500;

int tun_alloc(const char* devname);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server_ip>\n";
        return 1;
    }

    int tun_fd = tun_alloc("tun1");
    if (tun_fd < 0) {
        std::cerr << "[-] Failed to create TUN interface\n";
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        std::cerr << "Invalid server IP address\n";
        return 1;
    }

    pollfd fds[2];
    fds[0].fd = tun_fd;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    char buffer[BUFFER_SIZE];

    std::cout << "[Client] Running. Waiting for TUN and UDP events...\n";

    while (true) {
        int nready = poll(fds, 2, -1);
        if (nready < 0) {
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t nread = read(tun_fd, buffer, sizeof(buffer));
            if (nread < 0) {
                perror("read tun");
                break;
            }

            ssize_t n = sendto(sockfd, buffer, nread, 0,
                               (sockaddr*)&servaddr, sizeof(servaddr));
            if (n < 0) perror("sendto");

            std::cout << "[Client] Sent " << nread << " bytes from TUN to server\n";
        }

    
        if (fds[1].revents & POLLIN) {
            ssize_t n = recv(sockfd, buffer, sizeof(buffer), 0);
            if (n < 0) {
                perror("recv");
                break;
            }

            ssize_t written = write(tun_fd, buffer, n);
            if (written < 0) {
                perror("write tun");
            }

            std::cout << "[Client] Received " << n << " bytes from server and wrote to TUN\n";
        }
    }

    close(tun_fd);
    close(sockfd);
    return 0;
}