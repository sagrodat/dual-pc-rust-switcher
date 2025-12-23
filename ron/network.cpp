// network.cpp
#include "network.h"
#include <winsock2.h>

SOCKET SetupListener(int port) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    
    bind(s, (sockaddr*)&server, sizeof(server));
    
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
    
    return s;
}

bool ReceivedSignal(SOCKET s) {
    char buffer[16];
    sockaddr_in from;
    int fromLen = sizeof(from);
    
    int result = recvfrom(s, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
    return (result > 0);
}

void SendSignal(const char* ip, int port) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);
    
    const char* msg = "BACK";
    sendto(s, msg, strlen(msg), 0, (sockaddr*)&server, sizeof(server));
    closesocket(s);
    WSACleanup();
}