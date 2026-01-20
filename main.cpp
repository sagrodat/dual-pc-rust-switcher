// main.cpp
#include <Windows.h>
#include <iostream>
#include "config.h"
#include "network.h"
#include "utils.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) {
    switch(msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Set transparent background
            SetBkMode(hdc, TRANSPARENT);
            
            // Set text color (white)
            SetTextColor(hdc, RGB(255, 0, 0));
            
            // Set font
            HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");
            SelectObject(hdc, hFont);
            
            // Get window size
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Draw text centered
            DrawText(hdc, WINDOW_TEXT, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            DeleteObject(hFont);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lparam);
}

// Function to check if client is alive (Blocking for ~200ms)
bool CheckClientAvailable(SOCKET listener, const std::string& targetIP) {
    // 1. Send PING
    SendPacket(targetIP.c_str(), SIGNAL_PORT, "PING");
    
    // 2. Wait briefly for ACK (20 attempts * 10ms = 200ms timeout)
    for(int i = 0; i < 20; i++) {
        std::string response = ReceivePacket(listener);
        if(response == "ACK") {
            return true;
        }
        Sleep(10);
    }
    return false;
}

int main() {
    
    // Set console visibility
    HWND console = GetConsoleWindow();
    ShowWindow(console, CONSOLE_VISIBILITY);

    std::string hostname = GetHostname();
    bool isDirector = (hostname == PC1_HOSTNAME);
    bool isClient = (hostname == PC2_HOSTNAME);
    std::cout << "Running on: " << hostname << (isDirector ? " (Director)" : " (Client)") << std::endl;

    SOCKET listener = SetupListener(SIGNAL_PORT);
    std::cout << "UDP listener active on port " << SIGNAL_PORT << std::endl;

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "OverlayWindow";
    RegisterClass(&wc);

    HWND hwnd = NULL;
    HWND previousWindow = NULL; 
    bool isToggled = false;

    if(isDirector) {
        POINT pt;
        GetCursorPos(&pt); 
        HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);

        MONITORINFO mi = {};
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &mi);

        int width = mi.rcMonitor.right - mi.rcMonitor.left;
        int height = mi.rcMonitor.bottom - mi.rcMonitor.top;
        int window_width = 250;
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
        SetLayeredWindowAttributes(hwnd, 0, WINDOW_OPACITY, LWA_ALPHA); 
        ShowWindow(hwnd, SW_HIDE);
    }

    // main loop

    while (true) 
    {

        // ---------------------------------------------------------
        // 1. READ NETWORK SIGNALS
        // ---------------------------------------------------------

        std::string msg = ReceivePacket(listener);
        if (!msg.empty()) {
            // DIRECTOR LOGIC
            if(isDirector) {
                if(msg == "BACK" && isToggled) {
                    ShowWindow(hwnd, SW_HIDE);
                    if(previousWindow && IsWindow(previousWindow)) ForceFocus(previousWindow);
                    isToggled = false;
                    previousWindow = NULL;
                    std::cout << "Client returned control." << std::endl;
                }
            }
            // CLIENT LOGIC
            else if(isClient) {
                if(msg == "PING") {
                    // Director is asking if we are here. Reply immediately!
                    SendPacket(PC1_IP.c_str(), SIGNAL_PORT, "ACK");
                    std::cout << "Responded to Handshake PING" << std::endl;
                }
            }
        }

        // ---------------------------------------------------------
        // 2. HANDLE HOTKEYS
        // ---------------------------------------------------------

        if (GetAsyncKeyState(HOTKEY) & 0x0001) 
        {
            if(isDirector) 
            {
                if (!isToggled) {
                    std::cout << "Handshaking..." << std::endl;
                    if (CheckClientAvailable(listener, PC2_IP)) {
                        std::cout << "Client Confirmed! Switching..." << std::endl;
                        
                        isToggled = true;
                        previousWindow = GetForegroundWindow();
                        ShowWindow(hwnd, SW_SHOW);
                        ForceFocus(hwnd);

                        std::string cmd = "-switchControlToClient:" + PC2_IP;
                        ShellExecuteA(NULL, "open", INPUT_DIRECTOR_PATH, cmd.c_str(), NULL, SW_HIDE);
                    } else {
                        std::cout << "FAILED: Client not running or unreachable." << std::endl;
                        // Optional: Play a beep so you know it failed
                        Beep(500, 200); 
                    }
                } else {
                    // If we are already toggled and press hotkey, maybe force switch back?
                    // Usually the client sends "BACK", but this is a failsafe.
                    // Should not be possible since when toggled hotkey would be registered on CLIENT PC
                    // This means we are on DIRECTOR PC
                    isToggled = false; // fix state
                    ShowWindow(hwnd, SW_HIDE); // hide window
                }
            }
            else if (isClient) 
            {
                // Client just sends the BACK command
                SendPacket(PC1_IP.c_str(), SIGNAL_PORT, "BACK");
                
                std::string cmd = "-switchControlToClient:" + PC1_IP;
                ShellExecuteA(NULL, "open", INPUT_DIRECTOR_PATH, cmd.c_str(), NULL, SW_HIDE);
            }
        }

        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        Sleep(10);
    }

    return 0;
}