#pragma once

#include <wintun.h>

// Объявляем внешние указатели на функции wintun
extern WINTUN_CREATE_ADAPTER_FUNC* pCreateAdapter;
extern WINTUN_START_SESSION_FUNC* pStartSession;
extern WINTUN_RECEIVE_PACKET_FUNC* pReceivePacket;
extern WINTUN_RELEASE_RECEIVE_PACKET_FUNC* pReleaseReceivePacket;
extern WINTUN_ALLOCATE_SEND_PACKET_FUNC* pAllocateSendPacket;
extern WINTUN_SEND_PACKET_FUNC* pSendPacket;
extern WINTUN_END_SESSION_FUNC* pEndSession;

// Добавим typedef для функции удаления адаптера вручную:
typedef void (*WINTUN_DELETE_ADAPTER_FUNC)(WINTUN_ADAPTER_HANDLE Adapter);
extern WINTUN_DELETE_ADAPTER_FUNC* pDeleteAdapter;

extern WINTUN_ADAPTER_HANDLE adapter;
extern WINTUN_SESSION_HANDLE session;

bool tun_alloc();
void tun_close();