// network.h
#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>

SOCKET SetupListener(int port);
bool ReceivedSignal(SOCKET s);
void SendSignal(const char* ip, int port);

#endif