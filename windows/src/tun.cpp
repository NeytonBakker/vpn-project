#include "tun.h"
#include <iostream>

WINTUN_CREATE_ADAPTER_FUNC* pCreateAdapter = nullptr;
WINTUN_START_SESSION_FUNC* pStartSession = nullptr;
WINTUN_RECEIVE_PACKET_FUNC* pReceivePacket = nullptr;
WINTUN_RELEASE_RECEIVE_PACKET_FUNC* pReleaseReceivePacket = nullptr;
WINTUN_ALLOCATE_SEND_PACKET_FUNC* pAllocateSendPacket = nullptr;
WINTUN_SEND_PACKET_FUNC* pSendPacket = nullptr;
WINTUN_END_SESSION_FUNC* pEndSession = nullptr;
WINTUN_DELETE_ADAPTER_FUNC* pDeleteAdapter = nullptr;

WINTUN_ADAPTER_HANDLE adapter = nullptr;
WINTUN_SESSION_HANDLE session = nullptr;

HMODULE wintun = nullptr; // global

bool tun_alloc() {
    wintun = LoadLibraryW(L"Wintun.dll");
    if (!wintun) {
        std::cerr << "Failed to load Wintun.dll\n";
        return false;
    }

    pCreateAdapter = (WINTUN_CREATE_ADAPTER_FUNC*)GetProcAddress(wintun, "WintunCreateAdapter");
    pStartSession = (WINTUN_START_SESSION_FUNC*)GetProcAddress(wintun, "WintunStartSession");
    pReceivePacket = (WINTUN_RECEIVE_PACKET_FUNC*)GetProcAddress(wintun, "WintunReceivePacket");
    pReleaseReceivePacket = (WINTUN_RELEASE_RECEIVE_PACKET_FUNC*)GetProcAddress(wintun, "WintunReleaseReceivePacket");
    pAllocateSendPacket = (WINTUN_ALLOCATE_SEND_PACKET_FUNC*)GetProcAddress(wintun, "WintunAllocateSendPacket");
    pSendPacket = (WINTUN_SEND_PACKET_FUNC*)GetProcAddress(wintun, "WintunSendPacket");
    pEndSession = (WINTUN_END_SESSION_FUNC*)GetProcAddress(wintun, "WintunEndSession");
    pDeleteAdapter = (WINTUN_DELETE_ADAPTER_FUNC*)GetProcAddress(wintun, "WintunDeleteAdapter");

    // Проверяем только обязательные функции для работы
    if (!pCreateAdapter || !pStartSession || !pReceivePacket || !pReleaseReceivePacket ||
        !pAllocateSendPacket || !pSendPacket || !pEndSession) {
        std::cerr << "Failed to get necessary Wintun function addresses\n";
        return false;
    }

    adapter = (*pCreateAdapter)(L"MyVPNAdapter", L"MyVPN", nullptr);
    if (!adapter) {
        std::cerr << "Failed to create adapter\n";
        return false;
    }

    session = (*pStartSession)(adapter, 0x100000); // 1 MB buffer
    if (!session) {
        std::cerr << "Failed to start session\n";
        return false;
    }

    return true;
}

void tun_close() {
    if (session && pEndSession) {
        (*pEndSession)(session);
        session = nullptr;
    }
    if (adapter) {
        if (pDeleteAdapter) {
            (*pDeleteAdapter)(adapter);
        }
        adapter = nullptr;
    }
    if (wintun) {
        FreeLibrary(wintun);
        wintun = nullptr;
    }
    std::cout << "TUN interface closed\n";
}