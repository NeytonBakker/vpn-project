#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "tun.h"  

extern WINTUN_SESSION_HANDLE session;

// Èñïðàâëåííûå extern óêàçàòåëè íà ôóíêöèè ñ òî÷íûì ñîâïàäåíèåì òèïîâ èç wintun.h
extern BYTE* (__stdcall* pReceivePacket)(WINTUN_SESSION_HANDLE, DWORD*);
extern void(__stdcall* pReleaseReceivePacket)(WINTUN_SESSION_HANDLE, const BYTE*);
extern BYTE* (__stdcall* pAllocateSendPacket)(WINTUN_SESSION_HANDLE, DWORD);
extern void(__stdcall* pSendPacket)(WINTUN_SESSION_HANDLE, const BYTE*);

constexpr int PORT = 5555;
constexpr size_t BUFFER_SIZE = 1500;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: client.exe <server_ip>\n";
        return 1;
    }

    const char* server_ip = argv[1];

    if (!tun_alloc()) {
        std::cerr << "Failed to allocate tun interface\n";
        return 1;
    }

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "[-] socket failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    std::cout << "[Client] Running. Forwarding between WinTun and UDP\n";

    char buffer[BUFFER_SIZE];

    std::thread read_thread([&] {
        while (true) {
            DWORD packetSize = 0;
            BYTE* packet = pReceivePacket(session, &packetSize);
            if (packet && packetSize > 0) {
                sendto(sockfd, (const char*)packet, packetSize, 0,
                    (sockaddr*)&servaddr, sizeof(servaddr));
                std::cout << "[Client] Sent " << packetSize << " bytes to server\n";
                pReleaseReceivePacket(session, packet);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        });

    while (true) {
        sockaddr_in from{};
        int fromlen = sizeof(from);
        int len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
            (sockaddr*)&from, &fromlen);
        if (len > 0) {
            BYTE* packet = pAllocateSendPacket(session, len);
            if (packet) {
                memcpy(packet, buffer, len);
                pSendPacket(session, packet);
                std::cout << "[Client] Received " << len << " bytes from server\n";
            }
        }
    }

    closesocket(sockfd);
    WSACleanup();
    tun_close();
    return 0;
}
