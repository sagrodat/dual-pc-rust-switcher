#include <Windows.h>
#include <iostream>
#include <string>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

const std::string PC1_HOSTNAME = "DESKTOP-V45G59E";  // Director PC
const std::string PC2_HOSTNAME = "DESKTOP-VKLG84S";  // Client PC
const std::string PC1_IP = "192.168.0.46"; // Director IP
const std::string PC2_IP = "192.168.0.66"; // Client IP
const int SIGNAL_PORT = 9999;

// Window procedure - handles window messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lparam);
}

// Setup UDP listener (Director only)
SOCKET SetupListener(int port) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    
    bind(s, (sockaddr*)&server, sizeof(server));
    
    // Make non-blocking
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
    
    return s;
}

// Check for signal (Director only)
bool ReceivedSignal(SOCKET s) {
    char buffer[16];
    sockaddr_in from;
    int fromLen = sizeof(from);
    
    int result = recvfrom(s, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromLen);
    return (result > 0);
}

// Send signal to director (Client only)
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


// Get current PC hostname
std::string GetHostname() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "";
}

void SendKeyPress(WORD vkCode) {
    INPUT inputs[2] = {};
    
    // Key down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vkCode;
    inputs[0].ki.dwFlags = 0;
    
    // Key up
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vkCode;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    
    SendInput(2, inputs, sizeof(INPUT));
}


// Helper function to force focus
void ForceFocus(HWND hwnd) {
    // Attach to foreground thread
    HWND foreground = GetForegroundWindow();
    DWORD foregroundThread = GetWindowThreadProcessId(foreground, NULL);
    DWORD currentThread = GetCurrentThreadId();
    
    AttachThreadInput(currentThread, foregroundThread, TRUE);
    
    // Aggressive focus stealing
    BringWindowToTop(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    SetActiveWindow(hwnd);
    
    AttachThreadInput(currentThread, foregroundThread, FALSE);
}

int main() {
    // Auto-detect which PC we're on
    std::string hostname = GetHostname();
    bool isDirector = (hostname == PC1_HOSTNAME);
    bool isClient = (hostname == PC2_HOSTNAME);
    std::cout << "Running on: " << hostname << (isDirector ? " (Director)" : " (Client)") << std::endl;

    // Setup UDP listener (Director only)
    SOCKET listener = INVALID_SOCKET;
    if(isDirector) {
        listener = SetupListener(SIGNAL_PORT);
        std::cout << "UDP listener started on port " << SIGNAL_PORT << std::endl;
    }

    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OverlayWindow";
    wc.hbrBackground = CreateSolidBrush(RGB(255,0,0));
    RegisterClass(&wc);

    HWND hwnd = NULL;
    HWND previousWindow = NULL; 
    bool isToggled = false;

    // Create window (Director only)
    if(isDirector) {
        POINT pt;
        GetCursorPos(&pt); 
        HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);

        MONITORINFO mi = {};
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &mi);

        int width = mi.rcMonitor.right - mi.rcMonitor.left;
        int height = mi.rcMonitor.bottom - mi.rcMonitor.top;
        int window_width = 50;
        int window_height = 50;

        hwnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_APPWINDOW,
            "OverlayWindow",
            "Overlay",
            WS_POPUP,
            width-window_width,
            height-window_height,
            window_width,
            window_height,
            NULL, NULL, wc.hInstance, NULL
        );
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
        ShowWindow(hwnd, SW_HIDE);
    }

    while (true) 
    {
        // Check for signal from client (Director only)
        if(isDirector && listener != INVALID_SOCKET && hwnd && isToggled) {
            if(ReceivedSignal(listener)) {
                ShowWindow(hwnd, SW_HIDE);
                if(previousWindow && IsWindow(previousWindow)) {
                    ForceFocus(previousWindow);
                }
                isToggled = false;
                previousWindow = NULL;
                std::cout << "Signal received - window hidden!" << std::endl;
            }
        }

        // Key press detection
        if (GetAsyncKeyState(VK_END) & 0x0001) 
        {
            if(isDirector) 
            {
                std::cout << "Director: Toggling" << std::endl;
                isToggled = !isToggled;
               
                previousWindow = GetForegroundWindow();
        
                ShowWindow(hwnd, SW_SHOW);
                ForceFocus(hwnd);
                std::cout << "Window shown! Focus stolen!" << std::endl;

                std::string cmd = "-switchControlToClient:" + PC2_IP;
                ShellExecuteA(NULL, "open", 
                    "C:\\Program Files\\Input Director\\IDCmd.exe",
                    cmd.c_str(),
                    NULL, SW_HIDE);
                std::cout << "Switched to client" << std::endl;
            }
            else if (isClient) 
            {
                // Send signal to director BEFORE switching
                SendSignal(PC1_IP.c_str(), SIGNAL_PORT);
                std::cout << "Signal sent to director" << std::endl;
                
                std::string cmd = "-switchControlToClient:" + PC1_IP;
                ShellExecuteA(NULL, "open", 
                    "C:\\Program Files\\Input Director\\IDCmd.exe",
                    cmd.c_str(),
                    NULL, SW_HIDE);
                std::cout << "Switched to director" << std::endl;
            }
            else
            {
                std::cout << "UNKNOWN MACHINE" << std::endl;
            }
        }
        
        // Process window messages
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Sleep(10);
    }

    return 0;
}