#include "network.h"
#include <winsock2.h>
#include <iostream>

SOCKET SetupListener(int port) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    
    // Bind the socket
    bind(s, (sockaddr*)&server, sizeof(server));
    
    // Set to non-blocking mode
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
    
    return s;
}

std::string ReceivePacket(SOCKET s) {
    char buffer[1024]; // Buffer for incoming data
    sockaddr_in from;
    int fromLen = sizeof(from);
    
    // Try to receive data
    int result = recvfrom(s, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&from, &fromLen);
    
    if (result > 0) {
        buffer[result] = '\0'; // Null-terminate the string
        return std::string(buffer);
    }
    
    return ""; // Return empty if no data
}

void SendPacket(const char* ip, int port, const std::string& message) {
    // Note: In a production app you might want to init WSA once in main, 
    // but this is fine for this scale.
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);
    
    sendto(s, message.c_str(), message.length(), 0, (sockaddr*)&server, sizeof(server));
    
    closesocket(s);
}