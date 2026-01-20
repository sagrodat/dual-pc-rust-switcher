#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>
#include <string>

// Sets up a non-blocking UDP listener
SOCKET SetupListener(int port);

// Tries to receive a string message. Returns empty string "" if nothing received.
std::string ReceivePacket(SOCKET s);

// Sends a specific string message to an IP
void SendPacket(const char* ip, int port, const std::string& message);

#endif